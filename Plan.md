# BTCinC Plan

## File format
Lines separated with newlines

### Header/Tail
Protocol: `apsbtcincv0`
Previous file digest: SHA256 digest of previous file head, `TAIL` for genesis block

### Message
1. Message content followed by a newline
2. Timestamp of Message followed by a newline
3. Hash of message + previous hash + timestamp
- Note: If the message is the first in the file, the previous file digest will be used
4. RSA signature of hash
5. Public key of RSA signature

Separate messages with a blank line.
After last message, 2 blank lines.

### Head
1. `HEAD`
2. Proof of work: Content which, when added to previous message hash + timestamp of tail, has a first two hash digits of 0.
3. Hash of previous message hash + timestamp of tail + proof of work
4. RSA signature of hash
5. Public key of RSA signature
