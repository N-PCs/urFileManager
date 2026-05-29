# organizer.py

"""
Bulk File Organizer

This script provides a command-line utility to organize files within a specified
directory into subdirectories based on their file type. It supports a dry-run
mode for previewing changes, logs all operations to a file, and allows for
custom organization rules via an external 'config.json' file.
"""

# Standard library imports
import argparse
import json
import logging
import pathlib
import shutil
import sys

# Third-party imports
from tqdm import tqdm

def load_config(config_path: pathlib.Path) -> dict:
    """
    Loads and validates the organization rules from a JSON configuration file.

    This function attempts to open and parse the specified JSON file. It handles
    potential FileNotFoundError and json.JSONDecodeError, logging helpful
    error messages and exiting the script if the configuration is invalid or missing.

    Args:
        config_path (pathlib.Path): The path to the config.json file.

    Returns:
        dict: A dictionary containing the file type mappings.
    """
    try:
        with open(config_path, "r") as config_file:
            config_data = json.load(config_file)
            return config_data
    except FileNotFoundError:
        logging.error(f"Configuration file not found at: {config_path}")
        logging.error(
            "Please make sure 'config.json' exists in the same directory as the script."
        )
        # Exit with a non-zero status code to indicate a fatal error to the OS or calling scripts.
        sys.exit(1)
    except json.JSONDecodeError as e:
        logging.error(f"Error parsing configuration file: {config_path}")
        logging.error(
            f"The file contains invalid JSON. Please check the syntax. Details: {e}"
        )
        sys.exit(1)

def process_file(
    file_path: pathlib.Path,
    source_path: pathlib.Path,
    file_type_map: dict,
    dry_run: bool,
):
    """
    Processes a single file: determines its destination and moves it or simulates the move.

    This function is the core worker of the organization process. It finds the
    appropriate category for the file based on its extension, handles potential
    filename conflicts by renaming the file if necessary, and performs the
    actual move operation with error handling.

    Args:
        file_path (pathlib.Path): The path to the file to be processed.
        source_path (pathlib.Path): The root directory where organization is happening.
        file_type_map (dict): The dictionary of organization rules.
        dry_run (bool): If True, simulate the file move; otherwise, perform it.
    """
    # Convert extension to lowercase to ensure case-insensitive matching (e.g., .JPG matches .jpg).
    file_extension = file_path.suffix.lower()

    destination_folder_name = "Other"
    for category, extensions in file_type_map.items():
        if file_extension in extensions:
            destination_folder_name = category
            break

    destination_dir = source_path / destination_folder_name

    if dry_run:
        destination_file_path = destination_dir / file_path.name
        logging.info(
            f"[DRY RUN] Would move '{file_path.name}' -> '{destination_file_path}'"
        )
    else:
        # `parents=True` creates any missing parent directories.
        # `exist_ok=True` prevents an error if the directory already exists.
        destination_dir.mkdir(parents=True, exist_ok=True)

        destination_file_path = destination_dir / file_path.name
        counter = 1
        # This loop handles filename conflicts by appending a counter (e.g., file (1).txt).
        while destination_file_path.exists():
            logging.warning(f"Conflict: '{destination_file_path}' already exists.")
            new_filename = f"{file_path.stem} ({counter}){file_path.suffix}"
            destination_file_path = destination_dir / new_filename
            counter += 1

        try:
            shutil.move(file_path, destination_file_path)
            logging.info(f"Moved: '{file_path.name}' -> '{destination_file_path}'")
        except PermissionError as e:
            logging.error(f"Could not move '{file_path.name}'. Error: {e}")
        except Exception as e:
            logging.error(
                f"An unexpected error occurred while moving '{file_path.name}'. Error: {e}"
            )

def organize_directory(
    source_path: pathlib.Path, dry_run: bool, file_type_map: dict
):
    """
    Orchestrates the file organization process for a given directory.

    This function serves as the main entry point for the organization logic.
    It announces the operational mode (dry run or live), discovers all files
    in the source directory, and then delegates the processing of each file
    to the process_file function.

    Args:
        source_path (pathlib.Path): The directory to be organized.
        dry_run (bool): If True, simulate without moving files.
        file_type_map (dict): A dictionary mapping folder names to file extensions.
    """
    logging.info(f"Starting to organize directory: {source_path}")
    if dry_run:
        logging.info("--- DRY RUN MODE ENABLED: No files will be moved. ---")
    else:
        logging.warning("--- LIVE RUN MODE ENABLED: File system changes will be made. ---")

    # Use a list comprehension for a concise way to gather all files, ignoring subdirectories.
    files_to_process = [item for item in source_path.iterdir() if item.is_file()]

    for file_item in tqdm(files_to_process, desc="Organizing Files"):
        process_file(file_item, source_path, file_type_map, dry_run)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Organize files in a directory by their type.",
        epilog="Example: python organizer.py /path/to/downloads",
    )
    parser.add_argument(
        "source_directory", help="The path to the directory you want to organize."
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Simulate the organization without moving files.",
    )
    args = parser.parse_args()

    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s - %(levelname)s - %(message)s",
        handlers=[logging.FileHandler("organizer.log"), logging.StreamHandler(sys.stdout)],
    )

    # Build a robust path to config.json, ensuring it's found in the same directory as the script.
    # `__file__` is a special variable that holds the path to the current script.
    # `.parent` gets the directory containing the script.
    config_file_path = pathlib.Path(__file__).parent / "config.json"

    file_type_map_from_config = load_config(config_file_path)

    source_path = pathlib.Path(args.source_directory)

    if not source_path.is_dir():
        logging.error(
            f"Error: The provided path '{source_path}' is not a valid directory."
        )
        sys.exit(1)

    organize_directory(source_path, args.dry_run, file_type_map_from_config)
