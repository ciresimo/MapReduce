# Define the build directory
BUILD_DIR = build

# List of text files to be passed as arguments
TEXT_FILES = $(wildcard pg-*.txt)

# Build the project (only if necessary)
build:
	@rm -rf $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..
	@cd $(BUILD_DIR) && cmake --build .

# Run the program (without rebuilding)
run:
	@cd $(BUILD_DIR) && ./main $(addprefix $(CURDIR)/, $(TEXT_FILES))

