# Run3HFvalidation

## Keep your repositories and installations up to date

With the ongoing fast development, it can easily happen that updating the O<sup>2</sup> part of the validation
also requires updating the AliPhysics installation which then requires updating the alidist recipes as well.
Also when requesting changes in the main repository via a pull request, it is strongly recommended to update one's personal fork repository first,
apply the changes on top the main branch and rebuild the installation to make sure that the new commits can be seamlessly merged into the main repository.

All these maintenance steps can be fully automated using the `update_alio2.sh` bash script which takes care of keeping your (local and remote) repositories
and installations up to date with the latest development in the respective main branches.
This includes updating alidist, AliPhysics, O<sup>2</sup>, and this Run 3 validation code repository, as well as re-building your AliPhysics and O<sup>2</sup> installations via aliBuild.

All you need to do is to make sure that the strings in the `User's settings` block of the script correspond to your local setup and
adjust the activation switches, if needed, to change the list of steps to be executed.

You can then start the full update of your code and installations by running the script from any directory on your system:
```bash
bash <path to the Run3Analysisvalidation directory>/update_alio2.sh
```

If your repository is currently on a feature branch (different from the main branch), both the main branch and your feature branch will be updated from the main branch.
All your personal changes (committed and uncommitted) are preserved via rebasing and stashing.
Check the description of the script behaviour inside the script itself for more details.

You can easily extend the script to include any other local Git repository on your machine that you wish to be updated in the same way.

## [Heavy-flavour analyses](https://github.com/AliceO2Group/Run3Analysisvalidation/tree/master/codeHF)
