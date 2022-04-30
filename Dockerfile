# Dockerfile for psaikko/gbe-builder
# TODO: build and upload from github action

FROM ubuntu:20.04

RUN apt update && apt install -y \
    g++ \
    cmake \
    libalut-dev \
    libglfw3-dev \
    libglm-dev \
    libglew-dev
