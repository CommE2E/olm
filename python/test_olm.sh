#! /bin/bash

OLM="python -m olm"

ALICE_ACCOUNT=alice.account
ALICE_SESSION=alice.session
BOB_ACCOUNT=bob.account
BOB_SESSION=bob.session

rm $ALICE_ACCOUNT $BOB_ACCOUNT
rm $ALICE_SESSION $BOB_SESSION

$OLM create_account $ALICE_ACCOUNT
$OLM create_account $BOB_ACCOUNT
$OLM generate_keys $BOB_ACCOUNT 1

BOB_IDENTITY_KEY="$($OLM keys --json $BOB_ACCOUNT | jq -r .account_keys.curve25519)"
BOB_ONE_TIME_KEY="$($OLM keys --json $BOB_ACCOUNT | jq -r '.one_time_keys.curve25519|to_entries[0].value')"

$OLM outbound $ALICE_ACCOUNT $ALICE_SESSION "$BOB_IDENTITY_KEY" "$BOB_ONE_TIME_KEY"

echo "Hello world" | $OLM encrypt $ALICE_SESSION - - | $OLM inbound $BOB_ACCOUNT $BOB_SESSION - -
