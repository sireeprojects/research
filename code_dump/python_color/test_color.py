from colorama import init, Fore, Back, Style

# ALWAYS call init() first
init()

print(Fore.RED + "This is red text")
print(Fore.GREEN + "This is green text")
print(Back.YELLOW + Fore.BLACK + "Yellow background with black text")
print(Style.BRIGHT + Fore.CYAN + "This is bright cyan")
print(Style.DIM + Fore.MAGENTA + "This is dim magenta")

# CRITICAL: You must reset or the color will leak to the next line
print(Fore.RESET + Back.RESET + Style.RESET_ALL)
print("This is back to normal.")
