import argparse;    # for argument parsing 
import pathlib;     # for file paths
import sys;         # for exiting script
import shutil;      # provides high level file operations like moving files.
import logging;     # for Robust Script Output
from tqdm import tqdm   # for third party imports


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
def organize_directory(source_path: pathlib.Path, dry_run: bool):
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

    # listing the files to be processed
    files_to_process = [item for item in source_path.iterdir() if item.is_file()]


    # Iterate Over Directory Contents with pathlib
    for item in tqdm(files_to_process, desc="Organizing Files"):
        # The `if item.is_file():` check is no longer needed here because our
        # list comprehension has already pre-filtered for files.

        # ... (the rest of the loop logic remains exactly the same)
        file_extension = item.suffix
        
        destination_folder_name = 'Others'
        for category, extensions in FILE_TYPE_MAP.items():
            if file_extension in extensions:
                destination_folder_name = category
                break

        # Construct the full destination directory path using `source_path / destination_folder_name`.
        destination_dir = source_path / destination_folder_name

        # Implement Dry-Run Logic in a Python File Organizer
        if dry_run:
            # simulate the conflict resolution logic (e.g., adding '(1)').
            destination_file_path = destination_dir / item.name

            # logging the action to INFO level
            logging.info(f"[DRY RUN] Would move '{item.name}' -> '{destination_file_path}'")

            # confirming if dry run is active
            logging.info("--- DRY RUN MODE ENABLED: No files will be moved. ---")
        
        # Implement Live Mode for File Organization Script
        else:
            # confirming if live mode is active
            logging.warning("--- LIVE RUN MODE ENABLED: File system changes will be made. ---")

            # Create Destination Directories with Python Pathlib
            destination_dir.mkdir(parents=True, exist_ok=True)

            # Construct the Full Destination File Path in Python
            destination_file_path = destination_dir / item.name

            # If a file with the same name already exists, we find a new name.
            counter = 1

            while destination_file_path.exists():
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


if __name__=="__main__":        # this block of code runs only when execued from command line , this is our main entry point 
    parser = argparse.ArgumentParser(description="Organise files in a directory by thei type!")     # object creation for parsing commands and description provides brief summary of what program does

    parser.add_argument('source_directory', help='The path to directory you want to oragnise.')     # 'source_directory': This is the name we will use to access the argument's value later

    # Implement a Dry-Run Feature with argparse
    parser.add_argument('--dry-run', action='store_true', help='Simulate the organization without moving files.')

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
        logging.error(f"Error: The provided path '{source_path}' is not a valid directory.")

        sys.exit(1)     # exist the script and use 1 to denote error

    # printing confirmation message to user, showing the path that was provided
    organize_directory(source_path, args.dry_run)