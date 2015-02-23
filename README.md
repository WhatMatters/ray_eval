Rayeval is a poker evaluation library. It is based on the idea of the LUT-based 7-cards evaluator and 
Ray Wotton's (RayW) hand evaluator. But Rayeval can also evaluate 9-card hands, what makes it one of the 
fastes Omaha hand evaluator.


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

Shared memory
=============

Hand ranks files can be loaded to the shared memory (http://fscked.org/writings/SHM/shm-5.html).

But shared memory routines will only work if your OS is properly configured. In most operating systems default shared memory settings allows to use just few pages per segment.

In different OS there are different ways to change shared memory  settings. For example in Mac OS X you can find them in /etc/sysctl.conf and update them (without reboot) via sysctl command.

Note that max shared memory segment set in bytes and must be divisible by the size of the page. Total size of the shared memory segments system wide set in pages.

Mac OS X
--------
For example in Mac OS X you can update your shm settings (reboot is not required):

	sudo sysctl -w kern.sysv.shmmax=1598029824
	sudo sysctl -w kern.sysv.shmall=700000

Linux
-----
For example in Mac OS X you can update your shm settings (reboot is not required):

	sudo sysctl -w kernel.shmmax=1598029824
	sudo sysctl -w kernel.shmall=700000

License
=======
Rayeval is Copyright © 2013 Whatmatters, also some parts of the code shares copyright 
with Ray Wooten, Kevin L. Suffecool and probabply some of participants in the 2+2 forum thread 
(see http://archives1.twoplustwo.com/showflat.php?Cat=0&Number=8513906&page=0&fpart=13&vc=1)

It is free software, and may be redistributed under the terms specified in the
GNU GENERAL PUBLIC LICENSE Version 2 or higher (see LICENSE.txt).

For list of authors see AUTHORS.txt

This project is based on Cactus Kev's evaluator and inherits code from it, also it is based on the 
idea of LUT-based 7-cards evaluator and Ray Wotton's (RayW) hand evaluator and inherits code from it
and on the twoplustwo forum thread (http://archives1.twoplustwo.com/showflat.php?Cat=0&Number=8513906&page=0&fpart=13&vc=1)
in general.

The Ray Wooten, twoplustwo, and all derivations of it are made available 
through GPL v2 or later, based on the following:

(See http://pokerai.org/pf3/viewtopic.php?f=3&t=1451)

1) Also mykey1961 claimed that he first came with the idea of LUT-based 7-cards
hand evaluator, and that he makes this available in the public domain. We aren't sure
that he should be mentioned in AUTHORS.txt file, but he definetly must be mentioned here.

2) RayW posted his code on 2+2 udner GPL:
// HandRankSetup.cpp : Sets up the HandRank File for VERY fast Lookups
// by Ray Wotton and the 2+2 list My code is GPL, use it as you like
(see http://archives1.twoplustwo.com/showflat.php?Cat=0&Number=8513906&page=0&fpart=13&vc=1)

As RayW used Cactus Kev
(Cactus Kev's Eval Routine ref http://www.suffecool.net/poker/evaluator.html)
in his original work, and the resulting code was published and made
available under GPL, and stayed in the public domain without any complains,
we believe that the Cactus Kev code is hence made available under GPL. 

As a result, we make all of the code as part of this distribution 
available under GPL.

Authors
=======

Ivan Smirnov
Timothy N. Tsvetkov
Ray Wooten
Kevin L. Suffecool

Contributors
============

Sergey Pastukhov
Egor Azanov