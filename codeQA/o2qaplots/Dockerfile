FROM centos:7

SHELL ["/bin/bash", "-c"]

# Install ROOT
RUN yum install -y epel-release \
    && yum install -y \
    python3-3.6.8-18.el7.x86_64 \
    python36-root-6.22.06-1.el7.x86_64 \
    root-6.22.06-1.el7.x86_64 \
    && yum clean all \
    && rm -rf /var/cache/yum

COPY requirements.txt /requirements/requirements.txt

RUN pip3 install -r /requirements/requirements.txt --no-cache-dir

COPY . /o2qaplots
RUN pip3 install -e /o2qaplots/. --no-cache-dir
