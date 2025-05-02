# BTCinC Plan

## File format
Lines separated with newlines

### Header/Tail
Protocol: `apsbtcincv0`
Previous file digest: SHA256 digest of previous file head, `TAIL` for genesis block

### Message
1. Message content followed by a newline
2. Timestamp of Message
3. Hash of message + previous hash + timestamp
- Note: If the message is the first in the file, the previous file digest will be used
4. RSA signature of hash
5. Public key of RSA signature

Separate messages with a blank line.
After last message, 2 blank lines.

### Head
1. `HEAD`
2. Previous message hash
3. Timestamp of Tail
3. Proof of work: Content which, when added to 2 + 3, has a first digit of 0.
4. Hash of 2 + 3 + 4
5. RSA signature of hash
6. Public key of RSA signature
