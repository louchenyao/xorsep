FROM ubuntu:18.04

# install ssh server
RUN apt-get update && apt-get install -y openssh-server
RUN mkdir /var/run/sshd

# configure ssh authorized keys
RUN mkdir /root/.ssh
RUN wget -O - https://github.com/louchenyao.keys | tee /root/.ssh/authorized_keys

# install bazel
RUN apt-get -y install pkg-config zip g++ zlib1g-dev unzip python
RUN wget https://github.com/bazelbuild/bazel/releases/download/1.2.1/bazel-1.2.1-installer-linux-x86_64.sh
RUN chmod +x ./bazel-1.2.1-installer-linux-x86_64.sh && \
    ./bazel-1.2.1-installer-linux-x86_64.sh

# install tools
RUN apt-get -y install git

# clone and build to cache temporary files
# RUN git clone 
# RUN git 

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]