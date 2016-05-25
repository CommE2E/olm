/* Javascript parts of the group demo. To use, load group_demo.html in your
 * browser.
 */

function buttonAndTextElement(buttonLabel, textContent, clickHandler) {
    var el = document.createElement("div");

    var button = document.createElement("button");
    el.appendChild(button);
    button.appendChild(document.createTextNode(buttonLabel));

    var message_element = document.createElement("tt");
    el.appendChild(message_element);

    var content = document.createTextNode(textContent);
    message_element.appendChild(content);

    el.addEventListener("click", clickHandler, false);
    return el;
}

function DemoUser(name) {
    this.name = name;
    this.olmAccount = new Olm.Account();
    this.olmAccount.create();

    /* a list of the people in our chat */
    this.peers = [];

    /* for each peer, a one-to-one session - indexed by id key and created on
     * demand */
    this.peerSessions = {}

    /* for each peer, info on their sender session - indexed by id key and
     * session id */
    this.peerGroupSessions = {};

    /* our outbound group session */
    this.groupSession = undefined;

    /* a list of pending tasks */
    this.tasks = [];
    this.taskWorker = undefined;
}

DemoUser.prototype._progress = function(message) {
    var progress = this.progressElement;

    var message_element = document.createElement("pre");
    var start_content = document.createTextNode(message + "...");
    function start() {
        message_element.appendChild(start_content);
        progress.appendChild(message_element);
    }
    function done(res) {
        var done_content = document.createTextNode(message + "..." + res);
        message_element.replaceChild(done_content, start_content);
    }
    return {start:start, done:done};
};

DemoUser.prototype._do_tasks = function() {
    var self = this;
    var task = self.tasks.shift();
    var desc = task[0];
    var func = task[1];
    var callback = task[2];

    var p = self._progress(desc);
    p.start();

    function done() {
        p.done("Done");

        if (callback) {
            try {
                callback.apply(undefined, arguments)
            } catch (e) {
                console.error("Uncaught exception in callback", e.stack || e);
            }
        }

        start_tasks();
    }

    // sleep 50ms before actually doing the task
    self.taskWorker = window.setTimeout(function() {
        try {
            task[1](done);
        } catch (e) {
            console.error("Uncaught exception in task", e.stack || e);
            p.done("Failed: "+e);
            start_tasks();
        }
    }, 50);


    function start_tasks() {
        if (self.tasks.length == 0) {
            self.taskWorker = undefined;
            return;
        }

        self.taskWorker = window.setTimeout(self._do_tasks.bind(self), 50);
    }
}

/**
 * add a function "task" to this user's queue of things to do.
 *
 * task is called with a single argument 'done' which is a function to call
 * once the task is complete.
 *
 * 'callback' is called once the task is complete, with any arguments that
 * were passed to 'done'.
 */
DemoUser.prototype.addTask = function(description, task, callback) {
    this.tasks.push([description, task, callback]);
    if(!this.taskWorker) {
        this._do_tasks();
    }
};

DemoUser.prototype.addPeer = function(peer) {
    this.peers.push(peer);
};

DemoUser.prototype.getIdKey = function() {
    var keys = JSON.parse(this.olmAccount.identity_keys());
    return keys.curve25519;
};

DemoUser.prototype.generateKeys = function(callback) {
    var self = this;
    this.addTask("generate one time key", function(done) {
        self.olmAccount.generate_one_time_keys(1);
        done();
    }, callback);
};

DemoUser.prototype.getOneTimeKey = function() {
    var self = this;
    var keys = JSON.parse(self.olmAccount.one_time_keys())
        .curve25519;
    for (key_id in keys) {
        if (keys.hasOwnProperty(key_id)) {
            return keys[key_id];
        }
    }
    throw new Error("No one-time-keys generated");
};

/* ************************************************************************
 *
 * one-to-one messaging
 */

/**
 * retrieve, or initiate, a one-to-one session to a given peer
 */
DemoUser.prototype.getPeerSession = function(peer, callback) {
    var self = this;
    var peerId = peer.getIdKey();
    if (this.peerSessions[peerId]) {
        callback(this.peerSessions[peerId]);
        return;
    }

    this.addTask("get peer keys", function(done) {
        key = peer.getOneTimeKey();
        done(key);
    }, function(ot_key) {
        self.addTask("create peer session", function(done) {
            var session = new Olm.Session();
            session.create_outbound(self.olmAccount, peerId, ot_key);
            self.peerSessions[peerId] = session;
            done(session);
        }, callback);
    });
};

/**
 * encrypt a one-to-one message and prepare it for sending to a peer
 */
DemoUser.prototype.sendToPeer = function(peer, message, callback) {
    var self = this;
    this.getPeerSession(peer, function(session) {
        self.addTask("encrypt one-to-one message", function(done) {
            var encrypted = session.encrypt(message);
            var packet = {
                sender_key: self.getIdKey(),
                ciphertext: encrypted,
            };
            var json = JSON.stringify(packet);

            var el = buttonAndTextElement("send", json, function(ev) {
                peer.receiveOneToOne(json);
            });
            self.cipherOutputDiv.appendChild(el);
            done();
        }, callback);
    });
};

/**
 * handler for receiving a one-to-one message
 */
DemoUser.prototype.receiveOneToOne = function(jsonpacket) {
    var self = this;
    var el = buttonAndTextElement("decrypt", jsonpacket, function(ev) {
        var sender = JSON.parse(jsonpacket).sender_key;
        self.decryptOneToOne(jsonpacket, function(result) {

            var el2 = document.createElement("tt");
            el.appendChild(el2);

            var content = document.createTextNode(" -> "+result);
            el2.appendChild(content);

            var body = JSON.parse(result);

            // create a new inbound session if we don't yet have one
            if (!self.peerGroupSessions[sender] ||
                   !self.peerGroupSessions[sender][body.session_id]) {
                self.createInboundSession(
                    sender, body.session_id, body.message_index, body.session_key
                );
            }
        });
    });
    this.cipherInputDiv.appendChild(el);
};

/**
 * add a task to decrypt a one-to-one message. Calls the callback with the
 * decrypted plaintext
 */
DemoUser.prototype.decryptOneToOne = function(jsonpacket, callback) {
    var self = this;
    self.addTask("decrypt one-to-one message", function(done) {
        var packet = JSON.parse(jsonpacket);
        var peerId = packet.sender_key;

        var session = self.peerSessions[peerId];
        var plaintext;
        if (session) {
            plaintext = session.decrypt(packet.ciphertext.type, packet.ciphertext.body);
            done(plaintext);
            return;
        }

        if (packet.ciphertext.type != 0) {
            throw new Error("Unknown one-to-one session");
        }

        session = new Olm.Session();
        session.create_inbound(self.olmAccount, packet.ciphertext.body);
        self.peerSessions[peerId] = session;
        plaintext = session.decrypt(packet.ciphertext.type, packet.ciphertext.body);
        done(plaintext);
    }, callback)
};

/* ************************************************************************
 *
 * group messaging
 */


/**
 * retrieve, or initiate, an outbound group session
 */
DemoUser.prototype.getGroupSession = function() {
    if (this.groupSession) {
        return this.groupSession;
    }

    this.groupSession = new Olm.OutboundGroupSession();
    this.groupSession.create();

    var keymsg = {
        "session_id": this.groupSession.session_id(),
        "session_key": this.groupSession.session_key(),
        "message_index": this.groupSession.message_index(),
    };
    var jsonmsg = JSON.stringify(keymsg);

    for (var i = 0; i < this.peers.length; i++) {
        var peer = this.peers[i];
        this.sendToPeer(peer, jsonmsg);
    }

    return this.groupSession;
};

/**
 * add a task to create an inbound group session
 */
DemoUser.prototype.createInboundSession = function(
    peer_id, session_id, message_index, session_key, callback
) {
    var self = this;
    this.addTask("init inbound session", function(done) {
        session = new Olm.InboundGroupSession();
        session.create(message_index, session_key);
        if (!self.peerGroupSessions[peer_id]) {
            self.peerGroupSessions[peer_id] = {};
        }
        self.peerGroupSessions[peer_id][session_id] = session;
        done(session);
    }, callback);
};

/**
 * handler for receiving a group message
 */
DemoUser.prototype.receiveGroup = function(jsonpacket) {
    var self = this;
    var el = buttonAndTextElement("decrypt", jsonpacket, function(ev) {
        self.decryptGroup(jsonpacket, function(result) {
            var el2 = document.createElement("tt");
            el.appendChild(el2);

            var content = document.createTextNode(" -> "+result);
            el2.appendChild(content);
        });
    });
    this.groupInputDiv.appendChild(el);
};

/**
 * add a task to decrypt a received group message. Calls the callback with the
 * decrypted plaintext
 */
DemoUser.prototype.decryptGroup = function(jsonpacket, callback) {
    var self = this;
    this.addTask("decrypt group message", function(done) {
        var packet = JSON.parse(jsonpacket);

        var sender = packet.sender_key;
        var session_id = packet.session_id;

        var peer_sessions = self.peerGroupSessions[sender];
        if (!peer_sessions) {
            throw new Error("No sessions for sender "+sender);
        }

        var session = peer_sessions[session_id];
        if (!session) {
            throw new Error("Unknown session id " + session_id);
        }

        var plaintext = session.decrypt(packet.body);
        done(plaintext);
    }, callback);
};



/**
 * add a task to encrypt, and prepare for sending, a group message.
 *
 * Will create a group session if necessary
 */
DemoUser.prototype.encrypt = function(message) {
    var self = this;
    var session = this.getGroupSession();

    self.addTask("encrypt group message", function(done) {
        var encrypted = session.encrypt(message);
        var packet = {
            sender_key: self.getIdKey(),
            session_id: session.session_id(),
            body: encrypted,
        };
        var json = JSON.stringify(packet);

        var el = buttonAndTextElement("send", json, function(ev) {
            for (var i = 0; i < self.peers.length; i++) {
                var peer = self.peers[i];
                peer.receiveGroup(json);
            }
        });
        self.groupOutputDiv.appendChild(el);
        done();
    });
};



function initUserDiv(demoUser, div) {
    demoUser.progressElement = div.getElementsByClassName("user_progress")[0];
    demoUser.cipherOutputDiv = div.getElementsByClassName("user_cipher_output")[0];
    demoUser.cipherInputDiv = div.getElementsByClassName("user_cipher_input")[0];
    demoUser.groupOutputDiv = div.getElementsByClassName("group_output")[0];
    demoUser.groupInputDiv = div.getElementsByClassName("group_input")[0];

    var plain_input = div.getElementsByClassName("user_plain_input")[0];
    var encrypt = div.getElementsByClassName("user_encrypt")[0];

    encrypt.addEventListener("click", function() {
        demoUser.encrypt(plain_input.value);
    }, false);

}

function startDemo() {
    var user1 = new DemoUser();
    initUserDiv(user1, document.getElementById("user1"));
    user1.generateKeys();

    var user2 = new DemoUser();
    initUserDiv(user2, document.getElementById("user2"));
    user2.generateKeys();

    user1.addPeer(user2);
    user2.addPeer(user1);
}


document.addEventListener("DOMContentLoaded", startDemo, false);
