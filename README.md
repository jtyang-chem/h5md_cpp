# HDF5 CPP API example for h5md format

I wrote this because I found no manual for CPP API and lost in offical example's new/delete jungle.

And I don't want to use C API in a cpp program.

For now, we have
- nopbc
- write h5md trajectory
- flush (actually a close and re-open to avoid ill-flush in official method)
