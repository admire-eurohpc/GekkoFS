.PHONY: all

amd64:
	docker buildx build --platform amd64 -t gekkofs/linter:0.9.3 .

aarch64:
	docker buildx build --platform aarch64 -t gekkofs/linter:0.9.3 .

all:
	docker buildx build -t gekkofs/linter:0.9.3 .
