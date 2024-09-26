FROM --platform=linux/amd64 ubuntu:23.04 as builder
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libz-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y clang cmake make


ADD . /icer_compression
RUN mkdir -p /icer_compression/build
WORKDIR /icer_compression/build
RUN cmake ..
RUN make 

FROM --platform=linux/amd64 ubuntu:23.04

COPY --from=builder /icer_compression /icer_compression

COPY --from=builder /icer_compression/build/compress /

COPY --from=builder /icer_compression/boat.512.bmp /



# WORKDIR /icer_compression/build

# RUN ./compress_color 
