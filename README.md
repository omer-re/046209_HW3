# 046209_HW3
Basic TFTP server implementation



## How to Use

# to turn on server side:
./ttftps <port number>
for example:
./ttftps 12345



# on client side, only one at a time.
./tftp localhost <same port number> S outgoing_file.txt incoming_file.txt

for example:

./tftp localhost 12345 S outgoing_file.txt incoming_file.txt
