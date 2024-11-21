
Some wxWidgets samples need external data files and they expect to find
them in the current working directory and this works when the sample 
project is selected in the solution and run from MSVS. However, it can
be convenient to run these samples also directly from the folder where
their executable is. 

wxSamplesVCXProj Helper is a simple program that generates the command
files called in the post-build event to copy the sample data files to
the folder with its executable.

wxSamplesVCXProj Helper uses samples's bakefiles to obtain the list of
the data fiiles but also a project-specific hack (for internat sample)
since it does not support all bakefile features.
