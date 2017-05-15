#!/usr/bin/python
__author__ = 'jrschac'
# Title: Program 5 - Python Exploration
# Author: Jeffrey Schachtsick
# Course: CS344 - Operating Systems 1
# Last Update: 05/25/2016
# Description: Create 3 files in the same directory as this script with different names.
#   Each file will contain 10 random lowercase letter with no spaces, and the final
#   character will have a newline character.  As each random letter is discoverd, it is
#   output to display for the user.  After the letter stuff, select two random integers
#   between 1 and 42 inclusively.  Display these numbers and the product of the two.
# Libraries used: python 2.7, random

# Imports
import os
import re
import sys
import random

# Pre: There is a defined start and stop variables
# Description: returns a random number between start and stop values
# Post: Returns a random number
def randomizer(start, stop):
    """
    :param start:
    :param stop:
    :return: random number
    """
    # Source: https://docs.python.org/2/library/random.html
    # Get a random integer from start and stop
    random_number = random.randint(start, stop)
    # return that value
    return random_number

# Pre: Should have already created the 3 files
# Description: Prints out two random numbers and the product of the two
# Post: nothing
def partTwo():
    """
    :return: nothing
    """
    # Set variables
    start_low = 1
    stop_high = 42

    # Get first random number
    num_one = randomizer(start_low, stop_high)

    # Get second random number
    num_two = randomizer(start_low, stop_high)

    # Find the product of the two
    num_product = num_one * num_two

    # Display both numbers and their product
    print "Number One: ", num_one
    print "Number Two: ", num_two
    print "The product of One and Two: ", num_product

# Pre: nothing
# Description: management of creating 3 files with 10 random letters each
# Post: returns nothing
def partOne():
    """
    :return: nothing
    """
    # Set variables
    num_files = 3
    num_letters = 10
    start_low = 97
    stop_high = 122

    # Loop 3 times by creating a file
    for x in range(1, num_files + 1):
        # Concatenate the file with number and .txt
        # Source: http://stackoverflow.com/questions/6981495/how-can-i-concatenate-a-string-and-a-number-in-python
        filename = "file%d" %x + '.txt'
        print "Created file ", filename
        # Open a file with write permissions
        # Source: http://www.tutorialspoint.com/python/python_files_io.htm
        opened_file = open(filename, 'w')

        # Loop 10 times
        for y in range(0, num_letters):
            random_num = 0
            # Get a number from the randomizer
            random_num = randomizer(start_low, stop_high)

            # Choose letter from given number
            letter = chr(random_num)

            # print letter into file
            opened_file.write(letter)

            # print letter to display
            print letter

        # End of loop, add new line character to file
        opened_file.write("\n")

        # close the file
        #Source: http://www.tutorialspoint.com/python/python_files_io.htm
        opened_file.close()

        # Done with 3 files.



# Main management of program
def main():
    """
    :return: nothing
    """
    # Set variables

    # Function to create 3 files with 10 random letters each
    partOne()

    # Function to get two random integers, display those and the product
    partTwo()

    # DONE


main()