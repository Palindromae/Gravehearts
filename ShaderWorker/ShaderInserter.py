import os
import sys
import re

def insert_contents(original_file_path, origonal_file_name, marker_pattern, files_directory):
    # Regex pattern to match the insert marker
    pattern = re.compile(marker_pattern)
    
    print(origonal_file_name)

    # Temporary file path
    temp_file_path = os.path.join(files_directory, os.path.join("TEMP", origonal_file_name)) + '.tmp'
    origonal_file_file = os.path.join(original_file_path, origonal_file_name)
    with open(origonal_file_file) as original_file, \
         open(temp_file_path, 'w') as temp_file:
        
        for line in original_file:
            match = pattern.search(line)
            
            if match:
                # Extract the filename from the matched line
                filename = match.group(1)
                
                # Construct the full path to the file to insert
                file_to_insert_path = os.path.join(os.path.join(original_file_path, "Inserts"), filename)
                print(file_to_insert_path)
                # Check if the file exists
                if os.path.exists(file_to_insert_path):
                    # Read and write the contents of the file to insert
                    with open(file_to_insert_path, 'r') as file_to_insert:
                        temp_file.write("\n// START OF INSERTED TEXT\n")
                        temp_file.write(file_to_insert.read())
                        temp_file.write("\n// END OF INSERTED TEXT\n")
                else:
                    print(f"Warning: File {filename} does not exist.")
            else:
                # If no match, write the line as is
                temp_file.write(line)
        original_file.close()
        temp_file.close()

    if os.path.isfile(os.path.join(files_directory, os.path.join("TEMP", origonal_file_name))):
        os.remove(os.path.join(files_directory, os.path.join("TEMP", origonal_file_name)))
        
    # Rename the temporary file to the original file
    os.rename(temp_file_path, os.path.join(files_directory, os.path.join("TEMP", origonal_file_name)))


# Usage
marker_pattern = r'\/\/\#Insert ([\w.]+)'  

if len(sys.argv) >  1:
    FileToCopy = sys.argv[1]
   # VulkanBuild = sys.argv[1][0]
    print(sys.argv)
else:
    print("No arguments were provided.")

print(FileToCopy)
filename = os.path.basename(FileToCopy)
path = os.path.dirname(FileToCopy)

#id = OnAllowedList(filename)

print(f"inserting {filename}")
insert_contents(path, filename, marker_pattern, os.path.dirname(__file__))
#  os.system(VulkanBuild+toList[id])
