.PHONY: all

amd64:
	docker build --platform amd64 -t gekkofs/testing:0.9.3-rc1 .

aarch64:
	docker build --platform aarch64 -t gekkofs/testing:0.9.3-rc1 .

all:
	docker build -t gekkofs/testing:0.9.3-rc1 .
