EXE = dfg2config

$(EXE):
	g++ -o $(EXE) *.cpp

clean:
	rm -rf $(EXE) *.o
