QUICKSTART:
-----------
Recommended java version: 1.7 64Bit.
(minimum 1.6 32Bit, but 32Bit VM's have limitations regarding the maximum useable memory)

Default start:
cd /path/to/PDGFEnvironment
java -XX:NewRatio=1 -jar pdgf.jar -l demo-schema.xml -l demo-generation.xml -c -s

same as above, but using long version of commands:
cd /path/to/PDGFEnvironment
java -XX:NewRatio=1 -jar pdgf.jar -load demo-schema.xml -load demo-generation.xml -closeWhenDone -start 

Start generation with a specific scale factor:
java -XX:NewRatio=1 -jar pdgf.jar -l demo-schema.xml -l demo-generation.xml -c -s -sf 2000

To generate only one or more specific tables (Table names are case sensitive!) specify them as start command parameters:
java -XX:NewRatio=1 -jar pdgf.jar -l demo-schema.xml -l demo-generation.xml -c -s Customer

Recommended JVM memory settings Xms and Xmx for 85 workers:
java -Xms1300m -Xmx1300m -XX:NewRatio=1 -jar pdgf.jar -l demo-schema.xml -l demo-generation.xml -w 85 -c -s

Example: put all together (generate with 85 workers and scale factor 2000 only the table Customer):
java -Xms1300m -Xmx1300m -XX:NewRatio=1 -jar pdgf.jar -l demo-schema.xml -l demo-generation.xml -w 85 -c -s Customer -sf 2000


Parallelization
-------------------
Using the -w or -workers command, you can specify the number of parallel threads.
Using more threads also requires more memory, because of per thread caching and buffering of many things within the framework.
The memory consumption rises linearly with the amount of specified  workers.
If there is not enough memory available for the JVM to run PDGF properly, PDGF will warn you after you try to start the generation process and suggest better startup parameters.
You can either copy the parameters and restart PDGF yourself, or allow PDGF to restart (fork) itself using the recommended settings. Please follow the instructions on the screen.

Auditing and Reporting System
-----------------------------
To enable/disable the system, set this flag in the demo-schema.xml config file:
(1: active ; 0: disabled)
<property name="AuditingAndReportingSystemActive" type="double">1</property>

 
Exclusion of tables from the generation
----------------------------------------
If not all tables defined in the generation xml should be generated, the names of the tables which should be generated can be appended
to the 'start' option:
java -XX:NewRatio=1 -jar pdgf.jar -load demo-schema.xml -load demo-generation.xml -closeWhenDone -start Customer
In this case, only Customer will be generated.

To set the worker count to 1 and the scale factor to 10000, this would be the command to run:
java -XX:NewRatio=1 -jar pdgf.jar -load demo-schema.xml -load demo-generation.xml -closeWhenDone -workers 1 -sf 10000 -start


File encoding (charset) and system specific line.Seperators
------------------------------------------------------------
The default encoding used by PDGF is 'UTF-8' for all output files, if not overridden within the xxGeneration.xml files (Example: <output name="SomeOutputPlugin"> <charset>UTF-8</charset> </output>).
The default line separator is operating system dependent. Unix: LF, Windows: CRLF, MAC: CR

You can override this framework default behavior using two ways.
1. Start the JVM with an property -D<PropertyName>="<Value>":
	java -DDEFAULT_LINESEPARATOR="CRLF"c -jar pdgf.jar
	java -DDEFAULT_CHARSET="UTF-8" -jar pdgf.jar
	or both
	java -DDEFAULT_CHARSET="UTF-8" -DDEFAULT_LINESEPARATOR="CRLF" -jar pdgf.jar
2. create a file named "Constants.properties" in ..\PDGFEnvironment\ if it not already exists and add the two lines:
DEFAULT_LINESEPARATOR=CRLF
DEFAULT_CHARSET=UTF-8

Please see  ..\PDGFEnvironment\Documents\ConstantsCheatVars.pdf for a complete reference of supported properties an their values.
 
 
DETAILED EXPLANATION:
----------------------
To run PDGF, open a terminal, change into the PDGFEnvironment directory and run:

cd /path/to/PDGFEnvironment
java -XX:NewRatio=1 -jar pdgf.jar

This will open pdgf in interactive mode and optimizes the Heap space for young generation.
To exit the pdgf shell type 'exit'.
You can get a list of available commands with 'help' or  'h'. 
To get help for an specific command type: help <cmdName> where cmdName is the command name.
Example: 'help w'  will generate the following output:
    Description: number of threads/workers to be used (optional). Overrides automatic worker determination by cpu count
    Command priority: 10 (Default 10)
    Required parameters (min-max):1-1
    Usage:
    -w <number>
    or
    -workers <number>

To run the demo data generation, you have to enter at least these commands (in this order, l for load, s for start and w to set worker count):

l demo-schema.xml
l demo-generation.xml
s

The important thing is to first 'load' the two XML files in the given order and to 'start' when all other commands were entered.

After loading the two files and before starting generation, you can execute some other commands, for example:
sf <x> to change the scale factor (causes reloading of the XML files)
w <x> (to set the number of workers)
s <Table> <Table> ... <Table> (to generate only the specified tables)

For subsequent testing, it is better to use startup options, so the same commands do not need to be entered several times.
This can be done by appending the jar execution command line with commands listed by 'help' (of the form -{command-name})

To run the minimum example from above by only using the command line, this has to be done:
cd /path/to/PDGFEnvironment
java -XX:NewRatio=1 -Xmx512m -jar pdgf.jar -load demo-schema.xml -load demo-generation.xml -start
This command will generate the data and will leave you in the pdgf shell. 

To come back to the command prompt of your shell you can use 'closeWhenDone': 
java -XX:NewRatio=1 -Xmx512m -jar pdgf.jar -load demo-schema.xml -load demo-generation.xml -closeWhenDone -start
This time, pdgf will automatically exit after data generation finished and will return to the shell command prompt.


