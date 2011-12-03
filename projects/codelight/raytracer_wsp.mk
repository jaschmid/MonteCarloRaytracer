.PHONY: clean All

All:
	@echo ----------Building project:[ RaytraceCore - Debug ]----------
	@"$(MAKE)" -f "RaytraceCore.mk"
clean:
	@echo ----------Cleaning project:[ RaytraceCore - Debug ]----------
	@"$(MAKE)" -f "RaytraceCore.mk" clean
