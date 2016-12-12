
This project contains classes that are included in most MFC projects. I kept them separate, 
because more than one app referenced it.

The origianl code had the following stucture:

   workroot\projectname\work
   workroot\common\work

  So in the project the common class was referenced by adding "../../common/work" in the include configuration.

   This may complicate things on re-compiling the project, but it was neccery to incorporate classes evolving for decades.
 Yes, before git.   