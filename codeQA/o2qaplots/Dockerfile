FROM continuumio/miniconda:latest

COPY environment.yml .
RUN conda env create -f environment.yml

SHELL ["conda", "run", "-n", "o2qa", "/bin/bash", "-c"]

COPY requirements.txt .
RUN pip install -r requirements.txt

COPY . /o2qaplots

WORKDIR /o2qaplots
RUN pip3 install -e .
