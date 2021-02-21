#-------------------------------------------------------------------------------------------------------------
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License. See https://go.microsoft.com/fwlink/?linkid=2090316 for license information.
#-------------------------------------------------------------------------------------------------------------

FROM debian:buster
#FROM ubuntu:focal

# Avoid warnings by switching to noninteractive
ENV DEBIAN_FRONTEND=noninteractive

# Or your actual UID, GID on Linux if not the default 1000
#ARG USERNAME=juanma
#ARG USER_UID=1000
#ARG USER_GID=$USER_UID

# Configure apt and install packages
RUN apt-get update \
    && apt -y install --no-install-recommends apt-utils dialog 2>&1 \
    && apt -y install software-properties-common \
#    && apt-add-repository ppa:mutlaqja/ppa \
    && apt update \
    && apt upgrade -y \
#    && apt -y install indi-full libindi-dev \
#    && apt -y install swig libz-dev libz3-dev libcfitsio-dev libnova-dev \
#    && apt -y install python3 python3-pip python3-setuptools python3-dev \
    && apt -y install libz-dev libz3-dev \
    && apt -y install build-essential \
    && apt -y install git procps lsb-release \
    && apt -y install cmake gdb \
    && apt -y install libapr1-dev libaprutil1-dev gzip zip \
    && apt -y install locales \
    #
    #
    # Create a non-root user to use if preferred - see https://aka.ms/vscode-remote/containers/non-root-user.
#    && groupadd --gid $USER_GID $USERNAME \
#    && useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME \
    # [Optional] Add sudo support for non-root user
#    && apt-get install -y sudo \
#    && chmod 0440 /etc/sudoers.d/$USERNAME \
    #
    # Clean up
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

RUN locale-gen es_ES.UTF-8
RUN mkdir /var/log/gimon /etc/gimon
#RUN chown $USER_UID:$USER_GID /var/log/gimon /etc/gimon

ENV LC_ALL es_ES.UTF-8
ENV LANG es_ES.UTF-8
ENV LANGUAGE es_ES.UTF-8

RUN ln -sf /usr/share/zoneinfo/Europe/Madrid /etc/localtime

# Switch back to dialog for any ad-hoc use of apt-get
ENV DEBIAN_FRONTEND=dialog


