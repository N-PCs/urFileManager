# organizer.py

"""
urFileManager (Python CLI)

This script provides a command-line utility to organize files within a specified
directory into subdirectories based on their file type. It supports a dry-run
mode for previewing changes, logs all operations to a file, and allows for
custom organization rules via an external 'config.json' file.
"""

# Standard library imports
import argparse
import datetime
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

def escape_pdf_text(text: str) -> str:
    s = ""
    for c in text:
        if ord(c) < 128:
            if c in ('(', ')', '\\'):
                s += '\\'
            s += c
        else:
            s += '?'
    return s

def truncate_text(text: str, max_len: int) -> str:
    if len(text) <= max_len:
        return text
    return text[:max_len - 3] + "..."

def format_size(bytes_val: int) -> str:
    size = float(bytes_val)
    unit = "B"
    if size >= 1024:
        size /= 1024
        unit = "KB"
    if size >= 1024:
        size /= 1024
        unit = "MB"
    if size >= 1024:
        size /= 1024
        unit = "GB"
    return f"{size:.2f} {unit}"

def generate_pdf_report(report_path: pathlib.Path, moved_files: list, dry_run: bool):
    if not moved_files:
        return

    # Sort files by category and then filename to make the report structured
    moved_files = sorted(moved_files, key=lambda x: (x.get("category", ""), x.get("fileName", "")))

    # Calculate total space
    total_size = sum(f["fileSize"] for f in moved_files if f["status"] in ("Moved", "Renamed", "Dry Run Preview"))
    total_size_str = format_size(total_size)
    total_files = len(moved_files)

    # Calculate pagination
    rows_per_page_first = 25
    rows_per_page_subsequent = 32
    
    temp = total_files
    if temp <= rows_per_page_first:
        page_count = 1
    else:
        page_count = 1
        temp -= rows_per_page_first
        page_count += (temp + rows_per_page_subsequent - 1) // rows_per_page_subsequent

    time_str = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    contents = []
    file_idx = 0

    for p in range(page_count):
        ss = []
        # Faint border on page margins
        ss.append("0.9 0.9 0.9 RG 0.5 w 30 30 535 782 re S\n")

        if p == 0:
            # Document Header Card Box
            ss.append("0.95 0.95 0.98 rg 40 700 515 100 re f\n")
            ss.append("0.85 0.85 0.9 RG 1 w 40 700 515 100 re S\n")

            # Title Text
            title = "urFileManager - DRY RUN REPORT" if dry_run else "urFileManager - File Transfer Report"
            ss.append(f"BT /F2 18 Tf 0.1 0.1 0.25 rg 55 765 Td ({escape_pdf_text(title)}) Tj ET\n")

            # Metadata Lines
            ss.append(f"BT /F1 9 Tf 0.35 0.35 0.45 rg 55 745 Td (Report generated on: {escape_pdf_text(time_str)}) Tj ET\n")
            ss.append(f"BT /F1 9 Tf 0.35 0.35 0.45 rg 55 730 Td (Target Folder: {escape_pdf_text(str(report_path.parent))}) Tj ET\n")

            # Summary Info Strip
            status_text = "PREVIEW ONLY" if dry_run else "COMPLETED"
            ss.append(f"BT /F2 10 Tf 0.15 0.5 0.15 rg 55 712 Td (Status: {status_text}   |   Total Files: {total_files}   |   Total Space Organised: {escape_pdf_text(total_size_str)}) Tj ET\n")

            # Table Header Background and Text
            ss.append("0.9 0.9 0.92 rg 40 655 515 22 re f\n")
            ss.append("0.7 0.7 0.75 RG 1 w 40 655 515 22 re S\n")

            ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 48 662 Td (File Name) Tj ET\n")
            ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 268 662 Td (Category) Tj ET\n")
            ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 368 662 Td (Size) Tj ET\n")
            ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 458 662 Td (Status) Tj ET\n")

            start_y = 655
        else:
            # Subsequent Page Table Header
            ss.append("0.9 0.9 0.92 rg 40 770 515 22 re f\n")
            ss.append("0.7 0.7 0.75 RG 1 w 40 770 515 22 re S\n")

            ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 48 777 Td (File Name) Tj ET\n")
            ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 268 777 Td (Category) Tj ET\n")
            ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 368 777 Td (Size) Tj ET\n")
            ss.append("BT /F2 9 Tf 0.15 0.15 0.2 rg 458 777 Td (Status) Tj ET\n")

            start_y = 770

        items_on_this_page = rows_per_page_first if p == 0 else rows_per_page_subsequent
        rows_drawn = 0
        for row_idx in range(items_on_this_page):
            if file_idx >= total_files:
                break
            file = moved_files[file_idx]
            row_y = start_y - 20 - (row_idx * 20)
            rows_drawn += 1
            file_idx += 1

            # Alternate row zebra bg
            if row_idx % 2 == 1:
                ss.append(f"0.97 0.97 0.99 rg 40 {row_y} 515 20 re f\n")
            
            # Horizontal row divider line
            ss.append(f"0.9 0.9 0.9 RG 0.5 w 40 {row_y} m 555 {row_y} l S\n")

            text_y = row_y + 6

            # Render text
            ss.append(f"BT /F1 9 Tf 0.15 0.15 0.15 rg 48 {text_y} Td ({escape_pdf_text(truncate_text(file['fileName'], 38))}) Tj ET\n")
            ss.append(f"BT /F1 9 Tf 0.15 0.15 0.15 rg 268 {text_y} Td ({escape_pdf_text(truncate_text(file['category'], 15))}) Tj ET\n")
            ss.append(f"BT /F1 9 Tf 0.15 0.15 0.15 rg 368 {text_y} Td ({escape_pdf_text(format_size(file['fileSize']))}) Tj ET\n")

            # Status coloring
            st = file['status']
            if st == "Moved":
                ss.append(f"BT /F2 9 Tf 0.1 0.5 0.1 rg 458 {text_y} Td ({escape_pdf_text(st)}) Tj ET\n")
            elif st == "Renamed" or "Conflict" in st:
                ss.append(f"BT /F2 9 Tf 0.8 0.4 0.0 rg 458 {text_y} Td (Renamed) Tj ET\n")
            elif "Error" in st or "Failed" in st or "Could not" in st:
                ss.append(f"BT /F2 9 Tf 0.8 0.1 0.1 rg 458 {text_y} Td (Error) Tj ET\n")
            else:
                ss.append(f"BT /F1 9 Tf 0.2 0.2 0.7 rg 458 {text_y} Td ({escape_pdf_text(st)}) Tj ET\n")

        # Vertical columns lines
        final_y = start_y - 20 - (rows_drawn * 20)
        ss.append(f"0.85 0.85 0.85 RG 0.5 w 40 {final_y} m 40 {start_y} l S\n")
        ss.append(f"0.85 0.85 0.85 RG 0.5 w 260 {final_y} m 260 {start_y} l S\n")
        ss.append(f"0.85 0.85 0.85 RG 0.5 w 360 {final_y} m 360 {start_y} l S\n")
        ss.append(f"0.85 0.85 0.85 RG 0.5 w 450 {final_y} m 450 {start_y} l S\n")
        ss.append(f"0.85 0.85 0.85 RG 0.5 w 555 {final_y} m 555 {start_y} l S\n")

        # Table bottom border
        ss.append(f"0.7 0.7 0.75 RG 1 w 40 {final_y} m 555 {final_y} l S\n")

        # Footer
        ss.append(f"BT /F1 8 Tf 0.5 0.5 0.5 rg 270 45 Td (Page {p + 1} of {page_count}) Tj ET\n")

        contents.append("".join(ss))

    # PDF object assembly
    pdf_objects = []
    pdf_objects.append("<< /Type /Catalog /Pages 2 0 R >>")

    pages_kids = " ".join(f"{5 + p} 0 R" for p in range(page_count))
    pdf_objects.append(f"<< /Type /Pages /Kids [{pages_kids}] /Count {page_count} >>")
    pdf_objects.append("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>")
    pdf_objects.append("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold >>")

    for p in range(page_count):
        pdf_objects.append(f"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Resources << /Font << /F1 3 0 R /F2 4 0 R >> >> /Contents {5 + page_count + p} 0 R >>")

    for p in range(page_count):
        c_str = contents[p]
        pdf_objects.append(f"<< /Length {len(c_str)} >>\nstream\n{c_str}\nendstream")

    # Binary writing
    try:
        with open(report_path, "wb") as f:
            f.write(b"%PDF-1.4\n")
            
            obj_offsets = []
            current_offset = 9 # "%PDF-1.4\n" is 9 bytes

            for i, obj in enumerate(pdf_objects):
                obj_offsets.append(current_offset)
                obj_str = f"{i + 1} 0 obj\n{obj}\nendobj\n"
                obj_bytes = obj_str.encode("utf-8", errors="ignore")
                f.write(obj_bytes)
                current_offset += len(obj_bytes)

            xref_offset = current_offset
            f.write(b"xref\n")
            f.write(f"0 {len(pdf_objects) + 1}\n".encode())
            f.write(b"0000000000 65535 f\r\n")

            for offset in obj_offsets:
                f.write(f"{offset:010d} 00000 n\r\n".encode())

            f.write(b"trailer\n")
            f.write(f"<< /Size {len(pdf_objects) + 1} /Root 1 0 R >>\n".encode())
            f.write(b"startxref\n")
            f.write(f"{xref_offset}\n".encode())
            f.write(b"%%EOF\n")
    except Exception as e:
        logging.error(f"Failed to generate PDF Report. Error: {e}")

def process_file(
    file_path: pathlib.Path,
    source_path: pathlib.Path,
    file_type_map: dict,
    dry_run: bool,
    undo_log: list,
) -> dict:
    """
    Processes a single file: determines its destination and moves it or simulates the move.
    """
    file_extension = file_path.suffix.lower()
    destination_folder_name = "Other"
    for category, extensions in file_type_map.items():
        if file_extension in extensions:
            destination_folder_name = category
            break

    destination_dir = source_path / destination_folder_name
    file_size = file_path.stat().st_size if file_path.exists() else 0

    if dry_run:
        destination_file_path = destination_dir / file_path.name
        logging.info(
            f"[DRY RUN] Would move '{file_path.name}' -> '{destination_file_path}'"
        )
        return {
            "fileName": file_path.name,
            "category": destination_folder_name,
            "fileSize": file_size,
            "status": "Dry Run Preview"
        }
    else:
        destination_dir.mkdir(parents=True, exist_ok=True)
        destination_file_path = destination_dir / file_path.name
        counter = 1
        while destination_file_path.exists():
            logging.warning(f"Conflict: '{destination_file_path}' already exists.")
            new_filename = f"{file_path.stem} ({counter}){file_path.suffix}"
            destination_file_path = destination_dir / new_filename
            counter += 1

        try:
            shutil.move(file_path, destination_file_path)
            logging.info(f"Moved: '{file_path.name}' -> '{destination_file_path}'")
            undo_log.append({"original": str(file_path), "moved": str(destination_file_path)})
            return {
                "fileName": file_path.name,
                "category": destination_folder_name,
                "fileSize": file_size,
                "status": "Renamed" if counter > 1 else "Moved"
            }
        except PermissionError as e:
            logging.error(f"Could not move '{file_path.name}'. Error: {e}")
            return {
                "fileName": file_path.name,
                "category": destination_folder_name,
                "fileSize": file_size,
                "status": f"Permission Error: {e}"
            }
        except Exception as e:
            logging.error(
                f"An unexpected error occurred while moving '{file_path.name}'. Error: {e}"
            )
            return {
                "fileName": file_path.name,
                "category": destination_folder_name,
                "fileSize": file_size,
                "status": f"Error: {e}"
            }

def organize_directory(
    source_path: pathlib.Path, dry_run: bool, file_type_map: dict
):
    """
    Orchestrates the file organization process for a given directory.
    """
    logging.info(f"Starting to organize directory: {source_path}")
    if dry_run:
        logging.info("--- DRY RUN MODE ENABLED: No files will be moved. ---")
    else:
        logging.warning("--- LIVE RUN MODE ENABLED: File system changes will be made. ---")

    files_to_process = [item for item in source_path.iterdir() if item.is_file()]
    moved_files = []
    undo_log = []

    for file_item in tqdm(files_to_process, desc="Organizing Files"):
        res = process_file(file_item, source_path, file_type_map, dry_run, undo_log)
        moved_files.append(res)

    if not dry_run and undo_log:
        undo_path = source_path / ".organize_undo.json"
        with open(undo_path, "w") as f:
            json.dump({"moves": undo_log, "timestamp": str(datetime.datetime.now())}, f, indent=2)
        logging.info(f"Undo log saved to: {undo_path}")

    if moved_files:
        report_name = "organization_report_preview.pdf" if dry_run else "organization_report.pdf"
        report_path = source_path / report_name
        logging.info(f"Generating PDF report: {report_path}")
        generate_pdf_report(report_path, moved_files, dry_run)
        logging.info(f"PDF report successfully created at: {report_path}")


def revert_organization(source_path: pathlib.Path):
    """
    Reverts a previous organization by reading the undo log and moving files back.
    """
    undo_path = source_path / ".organize_undo.json"
    if not undo_path.exists():
        logging.error(f"No undo log found at: {undo_path}")
        logging.error("Cannot revert without an undo log.")
        return

    with open(undo_path, "r") as f:
        undo_data = json.load(f)

    moves = undo_data.get("moves", [])
    if not moves:
        logging.info("Undo log is empty. Nothing to revert.")
        return

    logging.info(f"Reverting {len(moves)} file(s)...")
    reverted = 0
    errors = 0

    for move in moves:
        original = pathlib.Path(move["original"])
        moved = pathlib.Path(move["moved"])

        if not moved.exists():
            logging.warning(f"File not found (skipped): {moved}")
            continue

        try:
            original.parent.mkdir(parents=True, exist_ok=True)
            shutil.move(moved, original)
            logging.info(f"Reverted: '{moved.name}' -> '{original}'")
            reverted += 1
        except Exception as e:
            logging.error(f"Failed to revert '{moved.name}': {e}")
            errors += 1

    logging.info(f"Revert complete. Reverted: {reverted}, Errors: {errors}")
    if errors == 0:
        undo_path.unlink()
        logging.info("Undo log removed.")

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
    parser.add_argument(
        "--revert",
        action="store_true",
        help="Revert a previous organization using the saved undo log.",
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

    if args.revert:
        revert_organization(source_path)
    else:
        organize_directory(source_path, args.dry_run, file_type_map_from_config)
