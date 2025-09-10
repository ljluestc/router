# Terraform Provider Generator Makefile

.PHONY: build run clean test install

# Build the generator
build:
	go build -o terraform-generator main.go

# Run the generator
run: build
	./terraform-generator $(PROVIDER) $(OUTPUT_DIR)

# Clean build artifacts
clean:
	rm -f terraform-generator
	rm -rf output/

# Run tests
test:
	go test -v ./...

# Install dependencies
install:
	go mod download
	go mod tidy

# Generate CloudPods provider
cloudpods: build
	./terraform-generator cloudpods ./output/cloudpods

# Generate Aviatrix provider
aviatrix: build
	./terraform-generator aviatrix ./output/aviatrix

# Generate Router Simulator provider
router-sim: build
	./terraform-generator router-sim ./output/router-sim

# Generate all providers
all: cloudpods aviatrix router-sim

# Help
help:
	@echo "Available targets:"
	@echo "  build      - Build the generator"
	@echo "  run        - Run the generator (requires PROVIDER and OUTPUT_DIR)"
	@echo "  clean      - Clean build artifacts"
	@echo "  test       - Run tests"
	@echo "  install    - Install dependencies"
	@echo "  cloudpods  - Generate CloudPods provider"
	@echo "  aviatrix   - Generate Aviatrix provider"
	@echo "  router-sim - Generate Router Simulator provider"
	@echo "  all        - Generate all providers"
	@echo "  help       - Show this help"
