FROM rootproject/root:6.22.02-ubuntu20.04

RUN apt update && apt install -y python3-pip

COPY requirements.txt .

RUN pip3 install -r requirements.txt

COPY . /o2qaplots

WORKDIR /o2qaplots

RUN pip3 install -e .