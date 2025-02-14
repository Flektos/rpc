FROM ubuntu:18.04
RUN apt update && apt install -y build-essential rpcbind libncurses-dev make
WORKDIR /app
COPY add.x add_client.c add_clnt.c add.h add_server.c add_svc.c add_xdr.c Makefile.add  ./ 
RUN make -f Makefile.add

