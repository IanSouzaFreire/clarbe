# List of file extensions to search for
extensions=("*.c" "*.cc" "*.cpp" "*.h" "*.hh" "*.hpp" "*.tpp")

# Command to run on each found file
command="clang-format -i -style=Google "

# Define the directory to search in (default is current directory)
search_dir="${1:-.}"

# List of directories to exclude (space-separated)
exclude_dirs=("build" "public" "private" "tomlplusplus" ".github" ".vscode" "private-test", "third-party", "curl")  # Add directories to exclude here

# Build the find command with exclusions
find_command="find \"$search_dir\""

# Add exclusions to the find command
for dir in "${exclude_dirs[@]}"; do
  find_command+=" -path \"$search_dir/$dir\" -prune -o"
done

# Add the file search criteria to the find command
find_command+=" -type f \( "

# Add each extension to the find command
for ext in "${extensions[@]}"; do
  find_command+=" -name \"$ext\" -o"
done

# Remove the last '-o' and close the parentheses
find_command="${find_command% -o} \) "

# Execute the find command and process the files
eval "$find_command" | while read -r file; do
  echo "Processing file: $file"
  $command "$file"
done