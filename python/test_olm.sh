#! /bin/bash

cd `dirname $0`

OLM="python -m olm"

ALICE_ACCOUNT=alice.account
ALICE_SESSION=alice.session
ALICE_GROUP_SESSION=alice.group_session
BOB_ACCOUNT=bob.account
BOB_SESSION=bob.session
BOB_GROUP_SESSION=bob.group_session

rm $ALICE_ACCOUNT $BOB_ACCOUNT
rm $ALICE_SESSION $BOB_SESSION
rm $ALICE_GROUP_SESSION $BOB_GROUP_SESSION

$OLM create_account $ALICE_ACCOUNT
$OLM create_account $BOB_ACCOUNT
$OLM generate_keys $BOB_ACCOUNT 1

BOB_IDENTITY_KEY="$($OLM identity_key $BOB_ACCOUNT)"
BOB_ONE_TIME_KEY="$($OLM one_time_key $BOB_ACCOUNT)"

$OLM outbound $ALICE_ACCOUNT $ALICE_SESSION "$BOB_IDENTITY_KEY" "$BOB_ONE_TIME_KEY"

echo "Hello world" | $OLM encrypt $ALICE_SESSION - - | $OLM inbound $BOB_ACCOUNT $BOB_SESSION - -


### group sessions

$OLM outbound_group $ALICE_GROUP_SESSION
$OLM group_credentials $ALICE_GROUP_SESSION | $OLM inbound_group $BOB_GROUP_SESSION
echo "Hello group" | $OLM group_encrypt $ALICE_GROUP_SESSION - - | $OLM group_decrypt $BOB_GROUP_SESSION
