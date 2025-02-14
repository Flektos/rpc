FROM ubuntu:18.04
RUN apt update && apt install -y build-essential rpcbind libncurses-dev make
COPY add.x data.h add_client.c add_clnt.c add.h add_server.c add_svc.c add_xdr.c Makefile.add  ./ 
RUN ls
RUN make -f Makefile.add
EXPOSE 111 2049


