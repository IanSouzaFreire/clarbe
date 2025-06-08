# Multi-OS, Multi-architecture Dockerfile to run build.sh for Debian, Arch, Red Hat (via CentOS), Gentoo, Slackware, NixOS, and Windows
# Builds both 64-bit and 32-bit (where feasible) for Linux OSes.
# Windows build runs build.sh in Windows container (64-bit assumed).

###########
# DEBIAN #
###########

# 64-bit Debian build
FROM debian:bookworm AS debian-x86_64

RUN dpkg --add-architecture i386 && \
    apt-get update && apt-get install -y build-essential gcc-multilib g++-multilib bash clang git && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app
RUN chmod +x build.sh ../specific/linux

# Run 64-bit build (default gcc target)
RUN ./build.sh ../specific/linux

# 32-bit Debian build
FROM debian:bookworm AS debian-x86

RUN dpkg --add-architecture i386 && \
    apt-get update && apt-get install -y build-essential gcc-multilib g++-multilib bash clang git && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app
RUN chmod +x build.sh ../specific/linux

# Set environment to build 32-bit binary via CFLAGS and cross compilation flags
ENV CFLAGS="-m32" CXXFLAGS="-m32" LDFLAGS="-m32"
RUN ./build.sh ../specific/linux


########
# ARCH #
########

# Arch 64-bit build
FROM archlinux:latest AS arch-x86_64

RUN pacman -Sy --noconfirm base-devel bash clang git && \
    pacman -Sy --noconfirm gcc-multilib lib32-glibc

WORKDIR /app
COPY . /app
RUN chmod +x build.sh ../specific/linux
RUN ./build.sh ../specific/linux

# Arch 32-bit build (cross compile with -m32)
FROM archlinux:latest AS arch-x86

RUN pacman -Sy --noconfirm base-devel bash clang git && \
    pacman -Sy --noconfirm gcc-multilib lib32-glibc

WORKDIR /app
COPY . /app
RUN chmod +x build.sh ../specific/linux
ENV CFLAGS="-m32" CXXFLAGS="-m32" LDFLAGS="-m32"
RUN ./build.sh ../specific/linux


#############
# RED HAT   #
# Using CentOS 8 #
#############

# Red Hat / CentOS 64-bit build
FROM centos:8 AS rhel-x86_64

RUN dnf install -y dnf-plugins-core && \
    dnf config-manager --set-enabled powertools && \
    dnf install -y gcc gcc-c++ make bash glibc-devel.i686 libstdc++-devel.i686 clang git && \
    dnf clean all

WORKDIR /app
COPY . /app
RUN chmod +x build.sh ../specific/linux
RUN ./build.sh ../specific/linux

# Red Hat 32-bit build (cross compile with -m32)
FROM centos:8 AS rhel-x86

RUN dnf install -y dnf-plugins-core && \
    dnf config-manager --set-enabled powertools && \
    dnf install -y gcc gcc-c++ make bash glibc-devel.i686 libstdc++-devel.i686 clang git && \
    dnf clean all

WORKDIR /app
COPY . /app
RUN chmod +x build.sh ../specific/linux
ENV CFLAGS="-m32" CXXFLAGS="-m32" LDFLAGS="-m32"
RUN ./build.sh ../specific/linux


############
# GENTOO   #
############

# Gentoo 64-bit build
FROM gentoo/portage:latest AS gentoo-x86_64

USER root
RUN emerge-webrsync && \
    emerge --sync && \
    emerge --oneshot --update --newuse sys-devel/gcc sys-devel/binutils sys-devel/make app-shells/bash dev-lang/clang dev-vcs/git

WORKDIR /app
COPY . /app
RUN chmod +x build.sh ../specific/linux
RUN ./build.sh ../specific/linux

# Gentoo 32-bit build (cross compile with -m32)
FROM gentoo/portage:latest AS gentoo-x86

USER root
RUN emerge-webrsync && \
    emerge --sync && \
    emerge --oneshot --update --newuse sys-devel/gcc sys-devel/binutils sys-devel/make app-shells/bash dev-lang/clang dev-vcs/git

WORKDIR /app
COPY . /app
RUN chmod +x build.sh ../specific/linux
ENV CFLAGS="-m32" CXXFLAGS="-m32" LDFLAGS="-m32"
RUN ./build.sh ../specific/linux


############
# SLACKWARE#
############

# Slackware 64-bit build (community image)
FROM alien/slackware:current AS slackware-x86_64

RUN slackpkg update && slackpkg install gcc make bash clang git

WORKDIR /app
COPY . /app
RUN chmod +x build.sh ../specific/linux
RUN ./build.sh ../specific/linux

# Slackware 32-bit build (cross compile with -m32)
FROM alien/slackware:current AS slackware-x86

RUN slackpkg update && slackpkg install gcc make bash clang git

WORKDIR /app
COPY . /app
RUN chmod +x build.sh ../specific/linux
ENV CFLAGS="-m32" CXXFLAGS="-m32" LDFLAGS="-m32"
RUN ./build.sh ../specific/linux


##########
# NIXOS  #
##########

# NixOS 64-bit build
FROM nixos/nix AS nixos-x86_64

RUN nix-env -iA nixos.gcc nixos.gnumake nixos.bash nixos.clang nixos.git

WORKDIR /app
COPY . /app
RUN chmod +x build.sh ../specific/linux
RUN ./build.sh ../specific/linux

# NixOS 32-bit build (cross compile with -m32) - may require nix config, assuming gcc supports it here
FROM nixos/nix AS nixos-x86

RUN nix-env -iA nixos.gcc nixos.gnumake nixos.bash nixos.clang nixos.git

WORKDIR /app
COPY . /app
RUN chmod +x build.sh ../specific/linux
ENV CFLAGS="-m32" CXXFLAGS="-m32" LDFLAGS="-m32"
RUN ./build.sh ../specific/linux



###########
# WINDOWS #
###########

# Windows 64-bit build
FROM mcr.microsoft.com/windows/servercore:ltsc2019 AS win-x86_64

SHELL ["powershell", "-Command"]

# Example: Download and install Git for Windows to get bash
RUN Invoke-WebRequest -Uri https://github.com/git-for-windows/git/releases/latest/download/Git-2.42.0-64-bit.exe -OutFile C:\\git-install.exe ; \
    Start-Process C:\\git-install.exe -ArgumentList '/VERYSILENT', '/NORESTART' -Wait ; \
    Remove-Item C:\\git-install.exe

# Install Clang (assuming a manual installation or a similar method)
# You may need to adapt this part to install Clang on Windows

WORKDIR C:\\app
COPY . C:\\app
# Switch to bash to run build.sh
RUN C:\\Program\ Files\\Git\\bin\\bash.exe -c "./build.sh" ../specific/win

# Windows 32-bit build is not common in Docker containers and not officially supported as container base.


# Notes:
# - To build and run a particular stage:
#   docker build --target debian-x86_64 -t build-debian-x64 .
#   docker run --rm build-debian-x64
#
# - Use the CFLAGS and environment variables to target 32-bit builds inside the container.
# - The build.sh script should respect environment variables CFLAGS, CXXFLAGS, LDFLAGS to build appropriately.
# - For Windows container, you need Windows Docker Host with Windows container mode enabled.
# - This Dockerfile assumes all source and build.sh are in current directory.