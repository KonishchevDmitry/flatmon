.PHONY: test clean

BUILDDIR := build

test:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake ..
	make -C $(BUILDDIR)
	CTEST_OUTPUT_ON_FAILURE=1 make -C $(BUILDDIR) test

clean:
	rm -rf $(BUILDDIR)
