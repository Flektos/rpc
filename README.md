## RPC PROJECT

### HOW TO RUN(PRE RPC)
1. git clone https://github.com/Flektos/rpc
2. cd rpc
3. touch [fileName].txt
4. make
5. ./main [fileName.txt] [processCount]

### RPC
1. Build the images
    * docker build -t rpc_server ./server
    * docker build -t rpc_client ./client
2. Create docker network
    * docker network create rpc_network
3. Start the server in interactive mode
    * docker run -it --rm --name server --network rpc_network rpc_server /bin/bash
4. Start the client in interactive mode
    * docker run -it --rm --name client --network rpc_network rpc_client /bin/bash

