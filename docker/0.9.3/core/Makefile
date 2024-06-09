.PHONY: all

amd64:
	docker buildx build --platform amd64 -t gekkofs/core:0.9.3-rc1 .

aarch64:
	docker buildx build --platform aarch64 -t gekkofs/core:0.9.3-rc1 .

all:
	docker buildx build -t gekkofs/core:0.9.3-rc1 .
