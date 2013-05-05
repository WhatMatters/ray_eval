Installation
============

Rayeval is distributed as a package and build on top of the C library and can be installed using setuptools.

Rayeval uses special data-files which are tricky indexed precalculated hand ranks. You need 7-card hand ranks file for
evaluating 7 card combinations (like in Holdem) and 9-card hand ranks file for evaluating 9 card combinations (for Omaha).

These files are generated at the inatall_data stage and can be re-generated using command:

	python setup.py install_data


Hand ranks data files generation
--------------------------------

Hand ranks files are generated when you installing the Rayeval or using:

	python setup.py install_data

By default generated files are installed to the shared directory in the default data_files location.
For example in Mac OS X it is:

	/System/Library/Frameworks/Python.framework/Versions/2.7/shared

Also files are linked to the package it was generated from (you can find a __rayeval_ranks_path__.txt file in
the package where path to the hand ranks files are stored). So you can access these files from the package.

### Hand rank files generation options

There are several command line arguments that you may use with:

	python setup.py install

or

	python setup.py install_data

--no-ranks-test will not run tests after generating ranks, false by default.

--without-build-ranks install Rayeval without generating hand ranks files.

--ranks-dir=/path/to/place/ sets path where to put generated hand ranks files.

--ranks-7-filename=rayeval_hand_ranks_7.dat name of the 7-card hand ranks file, rayeval_hand_ranks_7.dat is the default.

--ranks-9-filename=rayeval_hand_ranks_9.dat name of the 9-card hand ranks file, rayeval_hand_ranks_9.dat is the default.

--use-ranks-7=/path/to/rayeval_hand_ranks_7.dat links already generated 7-card hand ranks file to the package

--use-ranks-9=/path/to/rayeval_hand_ranks_7.dat links already generated 9-card hand ranks file to the package
