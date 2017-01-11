
This project contains classes that are included in most MFC projects. I kept them separate, 
because more than one app referenced it.

The original code had the following structure:

   workroot\projectname\work

   workroot\common\work

  So in the project the common class was referenced by adding "../../common/work" in the include configuration.

   This may complicate things on re-compiling the project, but it was necessary to incorporate classes evolving for decades.
 Yes, before git.   
