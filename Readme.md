# stassinopoulosari/BTCinC

*This is not an implementation of Bitcoin or any other cryptocurrency.*

*BTCinC* is my attempt at the final project for the Willamette University CS540 class. It is a simplified blockchain system written in ANSI C to apply various design patterns and algorithms discussed throughout the course.

Through the Github Actions CI/CD, I add one block to the blockchain every day. Before the program adds to the blockchain, it verifies the entire blockchain going back to the genesis block.

## Chain composition

Each block (in `bin/blocks/block_NUM.block`) is composed of a head, a chain, and a tail. The binary layout of the file is below:

### Protocol
The top of the file includes the string `apsbtcincv0` and a newline. Each element below includes a byte to signify what kind of element it is:

### Tail
The tail includes a hash of the signature of the head from the previous block: A byte `p` signifies a tail block, succeeded by the 32 bytes representing the signsture.

### Chain
After a delimiter (`|`), there are one or more chain entities, separated by the delimiter `|`. A chain consists of a `c` byte signifier, 8 bytes representing the size of the raw chain content, the content, 8 bytes representing the timestamp, 512 bytes representing the RSA signature of the hash of the chain, and 512 bytes representing the public key.

### Head
After a head delimiter `}`, we move on to the head. The head is signified by a `h` byte. Following this are 8 bytes representing the size of the proof-of-work and the raw proof-of-work itself. Following the proof-of-work, the timestamp is included using another 8 bytes before 512 bytes representing the signature and another 512 bytes representing the public key.

## Proof-of-Work
To reduce the load on the CI/CD system, the proof-of-work involves hashing the content of the chain head (including the proof-of-work and its size) such that the first two digits of the SHA256 hash are zeroes. Because of the variability of SHA256, this should take less than 64-128 attempts.

## RSA/SHA256
This blockchain uses insecure implementations of 4096-bit RSA and SHA256 from previous assignments, found in lib/bigrsa and lib/sha256, as well as a linked list, in lib/list_t.

## Conceit
I pared down several aspects of a blockchain implementation, including a timestemp server, the use of merkle trees for transaction storage, and variable proof-of-work. Making a single-user, centralized blockchain does not really make sense in any conceptual way.
