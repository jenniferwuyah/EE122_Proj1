import argparse
from os import listdir
from os.path import isfile, join

def main():

	# set up argument parser object
    parser = argparse.ArgumentParser()
    parser.add_argument('folder', type=str, \
		help=('Relative path to the folder to import.'))

    args    = parser.parse_args()
    folder = args.folder

    onlyfiles = [ join(folder,f) for f in listdir(folder) if (isfile(join(folder,f)) and ('stat' in f))]
    print(onlyfiles)


    for filename in onlyfiles:

	    # open the file
	    inId = open(filename, 'r')
	    num_lines = sum(1 for line in inId)
	    #print("oneliner? ", num_lines==1)
	    inId.seek(0)

	    total = 0
	    if num_lines ==1:
	    	while inId != None:
	    		line = inId.readline(8)
	    		if line == '': break;
	    		total += float(line)
	    else:
	    	total = sum(float(line) for line in inId)

	    print ("Total for file ", filename,  "is %.6f" %total)
	    inId.close()


##
# Boilerplate code to call main function
#
if __name__ == '__main__':
    main()