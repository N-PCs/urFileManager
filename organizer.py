 
import argparse;    # for argument parsing 
import pathlib;     # for file paths
import sys;         # for exiting script
import shutil;      # provides high level file operations like moving files.
import logging;     # for Robust Script Output


# defining a dictionary as the brain of the organiser
FILE_TYPE_MAP= {
    # Add Image File Extensions to a Dictionary
    "Images" : ['.jpeg', '.jpg', '.png', '.gif', '.svg'],
    
    # Add Document Category to File Organizer
    "Documents" :['.pdf', '.docx', '.txt', '.pptx', '.xlsx'],

    # Extend File Organizer with Audio File Support
    "Audio" : ['.mp3', '.wav', '.aac'],

    # Extend File Organizer to Handle Video Files
    "Video": ['.mp4', '.hevc', '.mov', '.avi', '.mkv'],

    # Add Archive File Category to Python Organizer
    "Archives" : ['.zip', '.rar', '.gz'],

    # Implement a Catch-All Category for a File Organizer
    "Others" : []
}

# Refactor Script Logic into a Main Function
def organize_directory(source_path: pathlib.Path):
    """
    Scans a directory and organizes files into subdirectories based on their type.

    This function is the main workhorse of the script. It will contain the logic
    for iterating through files, determining their type, creating destination
    folders, and moving the files.

    Args:
        source_path (pathlib.Path): The Path object representing the directory
                                    to be organized.
    """
    # The confirmation print statement is now moved inside our main function.
    # All future organizing logic will be added here. 
    # Send First Log Message with logging.info
    logging.info(f"Starting to organize directory: {source_path}")

    # Iterate Over Directory Contents with pathlib
    for item in source_path.iterdir():

        # Filter Directory Scan to Process Only Files
        if item.is_file():

            # Extract File Extension with pathlib.Path.suffix
            file_extension = item.suffix

            # Verify File Scan with a Print Feedback Loop
            print(f" - Found file: {item.name}, Extension: {file_extension}")

            # Set a Default Destination
            destination_folder_name='Others'

            # Implement File Classification Logic Using a Nested Loop and Python's 'in' Operator
            for category, extensions in FILE_TYPE_MAP.items():
                if file_extension in extensions:
                    # if match if found, update the our destination folder name 
                    destination_folder_name = category

                    # now break out of inner loop after finding a match 
                    break

            # Construct the full destination directory path using `source_path / destination_folder_name`.
            destination_dir = source_path / destination_folder_name

            # Create Destination Directories with Python Pathlib
            destination_dir.mkdir(parents=True, exist_ok=True)

            # Construct the Full Destination File Path in Python
            destination_file_path = destination_dir / item.name

            # If a file with the same name already exists, we find a new name.
            counter = 1

            while destination_file_path.exist():
                # logging in a WARNING
                logging.warning(f"Conflict: '{destination_file_path}' already exists.")

                # generate a new filename by inserting a counter before the extension
                new_filename = f"{item.stem} ({counter}){item.suffix}"

                # construct a new Path object for the new destination
                destination_file_path = destination_dir / new_filename

                # increment the counter
                counter+=1

            # Implement Error Handling with `try...except` for File Operations
            try:
                shutil.move(item, destination_file_path)
                # It will only be executed if the shutil.move() call succeeds.
                # Inside the `try` block, log a success message after a file is moved.
                logging.info(f"Moved: '{item.name}' -> '{destination_file_path}'")

            # Implement Specific Exception Logging in Python
            # Implement Robust Error Logging in a Python 
            except (FileExistsError, PermissionError) as e:
                # This is the detailed error log message.
                # object 'e' for the specific reason for the failure.
                logging.error(f"Could not move '{item.name}'. Error: {e}")
            # ADDITION: A catch-all for any other unexpected errors.
            except Exception as e:
                # This is a safety net. If an error other than FileExistsError
                # or PermissionError occurs, we still log it and prevent a crash.
                logging.error(f"An unexpected error occurred while processing '{item.name}'. Error: {e}")

            # add a print statement to verify our full destination path
            print(f"File: '{item.name}' -> Destination: '{destination_dir}'")

            # Move Files with shutil.move in Python
            shutil.move(item, destination_file_path)

            # add a print statement to give the user immediate feedback
            print(f"Moved: '{item.name}' -> '{destination_file_path}'")


        

if __name__=="__main__":        # this block of code runs only when execued from command line , this is our main entry point 
    parser = argparse.ArgumentParser(description="Organise files in a directory by thei type!")     # object creation for parsing commands and description provides brief summary of what program does

    parser.add_argument('source_directory', help='The path to directory you want to oragnise.')     # 'source_directory': This is the name we will use to access the argument's value later

    # line to trigger parsing process
    # takes user command arguments as attribute/input
    args=parser.parse_args()

    # Configure Python Logging with `basicConfig`
    logging.basicConfig(
        level=logging.INFO,                 # Define the format of the log messages.
        format='%(asctime)s - %(levelname)s - %(message)s',

        # specify the handlers, which are the destinations for the log messages.
        handlers=[
            # This handler writes log messages to a file named 'organizer.log'.
            logging.FileHandler("organizer.log"),
            # This handler writes log messages to the console (standard output).
            logging.StreamHandler(sys.stdout)
        ]        

    )

    # Convert Directory String to a Pathlib Object
    source_path = pathlib.Path(args.source_directory)

    # Validating a Directory Path
    if not source_path.exists() or not source_path.is_dir():
        print(f"Error: The path '{source_path}' does not exist or is not a directory!")

        sys.exit(1)     # exist the script and use 1 to denote error

    # printing confirmation message to user, showing the path that was provided
    organize_directory(source_path)



