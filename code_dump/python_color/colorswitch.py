import argparse
import sys
from colorama import init, Fore, Style

class Theme:
    def __init__(self, mode="dark"):
        # autoreset=True prevents color bleeding to the next line
        init(autoreset=True)
        self.set_theme(mode)

    def set_theme(self, mode):
        if mode == "light":
            self.header  = Fore.BLUE + Style.BRIGHT
            self.success = Fore.GREEN
            self.error   = Fore.RED + Style.BRIGHT
            self.info    = Fore.BLACK + Style.DIM
        elif mode == "dark":
            self.header  = Fore.CYAN + Style.BRIGHT
            self.success = Fore.GREEN + Style.BRIGHT
            self.error   = Fore.RED + Style.BRIGHT
            self.info    = Fore.WHITE + Style.DIM
        else:  # "none"
            self.header = self.success = self.error = self.info = ""

def main():
    # 1. Setup the Argument Parser
    parser = argparse.ArgumentParser(description="A script with selectable color themes.")
    
    # 2. Add the theme argument
    parser.add_argument(
        "--theme", 
        choices=["light", "dark", "none"], 
        default="dark",
        help="Set the terminal color theme (default: %(default)s)"
    )

    args = parser.parse_args()

    # 3. Handle Pipe detection (No color if redirecting to a file)
    current_theme = args.theme
    if not sys.stdout.isatty():
        current_theme = "none"

    # 4. Initialize the UI with the selected theme
    ui = Theme(current_theme)

    # 5. Application Logic
    print(f"{ui.header}--- System Update ---")
    print(f"{ui.info}Checking for updates...")
    print(f"{ui.success}Success: 0 packages upgraded.")
    
    if args.theme == "none":
        print("Note: Running in plain text mode.")

if __name__ == "__main__":
    main()
