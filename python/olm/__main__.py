#! /usr/bin/env python

import argparse
import json
import os
import sys
import yaml

from . import *

def build_arg_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument("--key", help="Account encryption key", default="")
    commands = parser.add_subparsers()

    create_account = commands.add_parser("create_account", help="Create a new account")
    create_account.add_argument("account_file", help="Local account file")

    def do_create_account(args):
        if os.path.exists(args.account_file):
            sys.stderr.write("Account %r file already exists" % (
                args.account_file,
            ))
            sys.exit(1)
        account = Account()
        account.create()
        with open(args.account_file, "wb") as f:
            f.write(account.pickle(args.key))

    create_account.set_defaults(func=do_create_account)

    keys = commands.add_parser("keys", help="List public keys for an account")
    keys.add_argument("account_file", help="Local account file")
    keys.add_argument("--json", action="store_true", help="Output as JSON")

    def do_keys(args):
        account = Account()
        with open(args.account_file, "rb") as f:
            account.unpickle(args.key, f.read())
        result = {
            "account_keys": account.identity_keys(),
            "one_time_keys": account.one_time_keys(),
        }
        try:
            if args.json:
                json.dump(result, sys.stdout, indent=4)
            else:
                yaml.safe_dump(result, sys.stdout, default_flow_style=False)
        except:
            pass

    keys.set_defaults(func=do_keys)

    sign = commands.add_parser("sign", help="Sign a message")
    sign.add_argument("account_file", help="Local account file")
    sign.add_argument("message_file", help="Message to sign")
    sign.add_argument("signature_file", help="Signature to output")

    def do_sign(args):
        account = Account()
        with open(args.account_file, "rb") as f:
            account.unpickle(args.key, f.read())
        with open_in(args.message_file) as f:
             message = f.read()
        signature = account.sign(message)
        with open_out(args.signature_file) as f:
             f.write(signature)

    sign.set_defaults(func=do_sign)


    generate_keys = commands.add_parser("generate_keys", help="Generate one time keys")
    generate_keys.add_argument("account_file", help="Local account file")
    generate_keys.add_argument("count", type=int, help="Number of keys to generate")

    def do_generate_keys(args):
        account = Account()
        with open(args.account_file, "rb") as f:
            account.unpickle(args.key, f.read())
        account.generate_one_time_keys(args.count)
        with open(args.account_file, "wb") as f:
            f.write(account.pickle(args.key))

    generate_keys.set_defaults(func=do_generate_keys)


    outbound = commands.add_parser("outbound", help="Create an outbound session")
    outbound.add_argument("account_file", help="Local account file")
    outbound.add_argument("session_file", help="Local session file")
    outbound.add_argument("identity_key", help="Remote identity key")
    outbound.add_argument("one_time_key", help="Remote one time key")

    def do_outbound(args):
        if os.path.exists(args.session_file):
            sys.stderr.write("Session %r file already exists" % (
                args.session_file,
            ))
            sys.exit(1)
        account = Account()
        with open(args.account_file, "rb") as f:
            account.unpickle(args.key, f.read())
        session = Session()
        session.create_outbound(
            account, args.identity_key, args.one_time_key
        )
        with open(args.session_file, "wb") as f:
            f.write(session.pickle(args.key))

    outbound.set_defaults(func=do_outbound)

    def open_in(path):
        if path == "-":
            return sys.stdin
        else:
            return open(path, "rb")

    def open_out(path):
        if path == "-":
            return sys.stdout
        else:
            return open(path, "wb")

    inbound = commands.add_parser("inbound", help="Create an inbound session")
    inbound.add_argument("account_file", help="Local account file")
    inbound.add_argument("session_file", help="Local session file")
    inbound.add_argument("message_file", help="Message", default="-")
    inbound.add_argument("plaintext_file", help="Plaintext", default="-")

    def do_inbound(args):
        if os.path.exists(args.session_file):
            sys.stderr.write("Session %r file already exists" % (
                args.session_file,
            ))
            sys.exit(1)
        account = Account()
        with open(args.account_file, "rb") as f:
            account.unpickle(args.key, f.read())
        with open_in(args.message_file) as f:
            message_type = f.read(8)
            message = f.read()
        if message_type != "PRE_KEY ":
            sys.stderr.write("Expecting a PRE_KEY message")
            sys.exit(1)
        session = Session()
        session.create_inbound(account, message)
        plaintext = session.decrypt(0, message)
        with open(args.session_file, "wb") as f:
            f.write(session.pickle(args.key))
        with open_out(args.plaintext_file) as f:
            f.write(plaintext)

    inbound.set_defaults(func=do_inbound)

    session_id = commands.add_parser("session_id", help="Session ID")
    session_id.add_argument("session_file", help="Local session file")

    def do_session_id(args):
        session = Session()
        with open(args.session_file, "rb") as f:
            session.unpickle(args.key, f.read())
        sys.stdout.write(session.session_id() + "\n")

    session_id.set_defaults(func=do_session_id)

    encrypt = commands.add_parser("encrypt", help="Encrypt a message")
    encrypt.add_argument("session_file", help="Local session file")
    encrypt.add_argument("plaintext_file", help="Plaintext", default="-")
    encrypt.add_argument("message_file", help="Message", default="-")

    def do_encrypt(args):
        session = Session()
        with open(args.session_file, "rb") as f:
            session.unpickle(args.key, f.read())
        with open_in(args.plaintext_file) as f:
            plaintext = f.read()
        message_type, message = session.encrypt(plaintext)
        with open(args.session_file, "wb") as f:
            f.write(session.pickle(args.key))
        with open_out(args.message_file) as f:
            f.write(["PRE_KEY ", "MESSAGE "][message_type])
            f.write(message)

    encrypt.set_defaults(func=do_encrypt)

    decrypt = commands.add_parser("decrypt", help="Decrypt a message")
    decrypt.add_argument("session_file", help="Local session file")
    decrypt.add_argument("message_file", help="Message", default="-")
    decrypt.add_argument("plaintext_file", help="Plaintext", default="-")

    def do_decrypt(args):
        session = Session()
        with open(args.session_file, "rb") as f:
            session.unpickle(args.key, f.read())
        with open_in(args.message_file) as f:
            message_type = f.read(8)
            message = f.read()
        if message_type not in {"PRE_KEY ", "MESSAGE "}:
            sys.stderr.write("Expecting a PRE_KEY or MESSAGE message")
            sys.exit(1)
        message_type = 1 if message_type == "MESSAGE " else 0
        plaintext = session.decrypt(message_type, message)
        with open(args.session_file, "wb") as f:
            f.write(session.pickle(args.key))
        with open_out(args.plaintext_file) as f:
            f.write(plaintext)

    decrypt.set_defaults(func=do_decrypt)

    outbound_group = commands.add_parser("outbound_group", help="Create an outbound group session")
    outbound_group.add_argument("session_file", help="Local group session file")
    outbound_group.set_defaults(func=do_outbound_group)

    group_credentials = commands.add_parser("group_credentials", help="Export the current outbound group session credentials")
    group_credentials.add_argument("session_file", help="Local outbound group session file")
    group_credentials.add_argument("credentials_file", help="File to write credentials to (default stdout)",
                                   type=argparse.FileType('w'), nargs='?',
                                   default=sys.stdout)
    group_credentials.set_defaults(func=do_group_credentials)

    group_encrypt = commands.add_parser("group_encrypt", help="Encrypt a group message")
    group_encrypt.add_argument("session_file", help="Local outbound group session file")
    group_encrypt.add_argument("plaintext_file", help="Plaintext file (default stdin)",
                               type=argparse.FileType('rb'), nargs='?',
                               default=sys.stdin)
    group_encrypt.add_argument("message_file", help="Message file (default stdout)",
                               type=argparse.FileType('w'), nargs='?',
                               default=sys.stdout)
    group_encrypt.set_defaults(func=do_group_encrypt)

    inbound_group = commands.add_parser(
        "inbound_group",
        help=("Create an inbound group session based on credentials from an "+
              "outbound group session"))
    inbound_group.add_argument("session_file", help="Local inbound group session file")
    inbound_group.add_argument("credentials_file",
                               help="File to read credentials from (default stdin)",
                               type=argparse.FileType('r'), nargs='?',
                               default=sys.stdin)
    inbound_group.set_defaults(func=do_inbound_group)

    group_decrypt = commands.add_parser("group_decrypt", help="Decrypt a group message")
    group_decrypt.add_argument("session_file", help="Local inbound group session file")
    group_decrypt.add_argument("message_file", help="Message file (default stdin)",
                               type=argparse.FileType('r'), nargs='?',
                               default=sys.stdin)
    group_decrypt.add_argument("plaintext_file", help="Plaintext file (default stdout)",
                               type=argparse.FileType('wb'), nargs='?',
                               default=sys.stdout)
    group_decrypt.set_defaults(func=do_group_decrypt)
    return parser

def do_outbound_group(args):
    if os.path.exists(args.session_file):
        sys.stderr.write("Session %r file already exists" % (
            args.session_file,
        ))
        sys.exit(1)
    session = OutboundGroupSession()
    with open(args.session_file, "wb") as f:
            f.write(session.pickle(args.key))

def do_group_encrypt(args):
    session = OutboundGroupSession()
    with open(args.session_file, "rb") as f:
        session.unpickle(args.key, f.read())
    plaintext = args.plaintext_file.read()
    message = session.encrypt(plaintext)
    with open(args.session_file, "wb") as f:
        f.write(session.pickle(args.key))
    args.message_file.write(message)

def do_group_credentials(args):
    session = OutboundGroupSession()
    with open(args.session_file, "rb") as f:
        session.unpickle(args.key, f.read())
    result = {
        'message_index': session.message_index(),
        'session_key': session.session_key(),
    }
    json.dump(result, args.credentials_file, indent=4)

def do_inbound_group(args):
    if os.path.exists(args.session_file):
        sys.stderr.write("Session %r file already exists\n" % (
            args.session_file,
        ))
        sys.exit(1)
    credentials = json.load(args.credentials_file)
    for k in ('message_index', 'session_key'):
        if not k in credentials:
            sys.stderr.write("Credentials file is missing %s\n" % k)
            sys.exit(1);

    session = InboundGroupSession()
    session.init(credentials['message_index'], credentials['session_key'])
    with open(args.session_file, "wb") as f:
        f.write(session.pickle(args.key))

def do_group_decrypt(args):
    session = InboundGroupSession()
    with open(args.session_file, "rb") as f:
        session.unpickle(args.key, f.read())
    message = args.message_file.read()
    plaintext = session.decrypt(message)
    with open(args.session_file, "wb") as f:
        f.write(session.pickle(args.key))
    args.plaintext_file.write(plaintext)

if __name__ == '__main__':
    parser = build_arg_parser()
    args = parser.parse_args()
    args.func(args)
