clean:
	rm -rf *.o
	rm -rf graph2skeleton
	rm -rf identifypipeline
	rm -rf identifypipeline_linear
reconfiguration.o: reconfiguration.cpp reconfiguration.hpp
	g++ -c reconfiguration.cpp
graph2skeleton.o: graph2skeletongraph.cpp reconfiguration.hpp
	g++ -c graph2skeletongraph.cpp
graph2skeleton: reconfiguration.o graph2skeletongraph.o
	g++ reconfiguration.o graph2skeletongraph.o -o graph2skeleton
	./graph2skeleton
identify_pipeline.o: identify_pipeline.cpp reconfiguration.hpp
	g++ -c identify_pipeline.cpp
identify_pipeline_linear.o: identify_pipeline_linear.cpp reconfiguration.hpp
	g++ -c identify_pipeline_linear.cpp
identifypipeline: reconfiguration.o identify_pipeline.o
	g++ reconfiguration.o identify_pipeline.o -o identifypipeline
	./identifypipeline
identifypipeline_linear: reconfiguration.o identify_pipeline_linear.o
	g++ reconfiguration.o identify_pipeline_linear.o -o identifypipeline_linear
	./identifypipeline_linear
