 
import argparse;    # for argument parsing 
import pathlib;     # for file paths

# defining a dictionary as the brain of the organiser
FILE_TYPE_MAP= {}

if __name__=="__main__":        # this block of code runs only when execued from command line , this is our main entry point 
    parser = argparse.ArgumentParser(description="Organise file sin a directory by thei type!")     # object creation for parsing commands and description provides brief summary of what program does

    parser.add_argument('source_directory', help='The path to directory you want to oragnise.')     # 'source_directory': This is the name we will use to access the argument's value later

    # line to trigger parsing process
    # takes user command arguments as attribute/input
    args=parser.parse_args()

    # printing confirmation message to user, showing the path that was provided
    print(f"Organising the files present in : {args.source_directory}")



