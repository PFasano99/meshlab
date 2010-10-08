
#include <sys/stat.h>

#include "ocme.h"
#include "impostor_definition.h" 
#include "../utils/memory_debug.h"
#include "../utils/timing.h"
#include "../utils/logging.h"

#include "../ooc_vector/ooc_chains.h"

//#include "gindex_chunk.h"

#include "import_ocm_ply.h"
#include "../ooc_vector/berkeleydb/ooc_chains_berkeleydb.hpp"


#include <wrap/io_trimesh/import_ply.h>
#include <wrap/io_trimesh/import_dae.h>
#include <wrap/io_trimesh/export_ply.h>
#include <vcg/math/quaternion.h>
#include <vcg/complex/trimesh/create/platonic.h>
#include <vcg/complex/trimesh/update/position.h>

#include <wrap/system/getopt.h>

#include "plane_box_intersection.h" 
#include <stdio.h>

#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


int getch(void)
{
struct termios oldt,
newt;
int ch;
tcgetattr( STDIN_FILENO, &oldt );
newt = oldt;
newt.c_lflag &= ~( ICANON | ECHO );
tcsetattr( STDIN_FILENO, TCSANOW, &newt );
ch = getchar();
tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
return ch;
}
int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}

#endif

//#include <gprof/src/google/profiler.h>

OOCEnv  cm;
OCME * meshona;

bool LoadTraMa(const char * filename, vcg::Matrix44f  & tr){
	float x,y,z,w;
	FILE * f = fopen( filename,"r");
	if(f==NULL) {
		printf("aln %s not found /n",filename); 
		return false;
	}
	for(int j = 0; j < 4; ++j){
		fscanf(f,"%f %f %f %f",&x,&y,&z,&w);
		tr[j][0] = x;  tr[j][1] = y; tr[j][2] = z; tr[j][3] = w;
	}
	fclose(f);
	return true;
};

bool LoadAln(const char * filename,std::vector<std::pair<std::string,vcg::Matrix44f> > & aln){
	int nm;
	char name[65536];
	char litter[255];
	float x,y,z,w;
	vcg::Matrix44f tr;
	FILE * f = fopen( filename,"r");
	if(f==NULL) {
		printf("aln %s not found /n",filename); 
		return false;
			}
	printf("aln file opened /n"); 
	fscanf(f,"%d",&nm);

	for(int i = 0; i < nm; ++i){
		if (fscanf(f,"%s",&name[0]) != 1 ) { return false;}
		fgets(litter,255,f);
		fgets(litter,255,f);

		for(int j = 0; j < 4; ++j){
			fscanf(f,"%f %f %f %f",&x,&y,&z,&w);
			tr[j][0] = x;  tr[j][1] = y; tr[j][2] = z; tr[j][3] = w;
		}

		aln.push_back(std::pair<std::string,vcg::Matrix44f>());
		aln.back().first = std::string(name);
		aln.back().second = tr;
	}

	fclose(f);
	return true;
};

bool cb(const int pos, const char * str ){
	printf("%s %d \r",str,pos);
	return true;
}

void AddImpostors(char * ocmFile){
		OCME * meshona = new OCME();
		meshona->Open(ocmFile);
		meshona->ComputeImpostors();
		meshona->Close(true);
}

void UsageExit(){
	printf("Usage ocm_build.exe [options] [*.aln | *.ply]...\
		   -f : database name\n \
		   -c : add per vertex color if present \
		   -o : owerwrite if already exists \
		   -m : RAM cache \
		   -p : DB page size (bytes) \n");

	exit(0);
}

struct Sample{
        Sample(unsigned int nf,float tim, unsigned int nc,
               int _a0,int _a1,int _a2,int _a3,int _a5 ):n_faces(nf),time(tim),n_cells(nc),a0(_a0),a1(_a1),a2(_a2),a3(_a3),a5(_a5){}
	unsigned int n_faces;
	unsigned int n_cells;
	float time;
        unsigned int a0,a1,a2,a3,a5;
};

std::vector<Sample> samples;

void PrintFacesSec(){
	lgn->Append("faces * secs");lgn -> Push();
	sprintf(lgn->Buf(),"#faces \t time \t fxs \t #cells");lgn->Push();
	for(unsigned int y = 0; y < samples.size(); ++y){
                sprintf(lgn->Buf(),"%10d \t %7.3f \t %7.3f \t %10d \t %10d \t %10d \t %10d \t %10d \t %10d \n",
                        samples[y].n_faces,samples[y].time,samples[y].n_faces/samples[y].time,samples[y].n_cells,
                        samples[y].a0,samples[y].a1,samples[y].a2,samples[y].a3,samples[y].a5);
		lgn->Push();
	}
}


bool Confirm(){
	printf("A key has been hit. This means you want to stop adding meshes.");
	printf("Do you confirm closing the databse now (it will be valid) [y/n]?");
	int answ;
	do{answ = getch();} while( ( answ != 'n') && (answ != 'y') && (answ != 'f'));
	if(answ=='f')
		meshona->oce.SaveData();
	return (answ == 'y');
}

bool Interrupt(){
	 
	if( kbhit() && Confirm())
		return true;
	return false;
}


//
//void AddFromDisk(OCME * ocme, std::vector<std::string> files){
//
//		for(unsigned int i = 0; i < files.size(); ++i){
//
//				std::string name = files[i];
//				unsigned int wh = name.find(std::string(".aln"));
//
//				if(wh == name.length()-4)
//				// it is an aln file
//				{
//					std::vector<std::pair<std::string,vcg::Matrix44f> > aln;
//					LoadAln(name.c_str(),aln);
//
//					for(unsigned int idm = 0;  idm<   aln.size() ;++idm){
//						vcgMesh m;
//						vcg::tri::io::ImporterPLY<vcgMesh>::Open(m,aln[idm].first.c_str());
//						vcg::tri::UpdatePosition<vcgMesh>::Matrix(m,aln[idm].second);
//						if(!m.face.empty()){
//							ocme->AddMesh(m);
//						}
//					}
//				}
//				else
//					// It is a ply file
//				{
//					vcgMesh m;
//					struct stat buf;
//					stat(name.c_str(),&buf);
//                                        if(buf.st_size < 200 * (1<<20)){// if the file is less that 200MB load the mesh in memory and then add it
//
//						int mask = 0;
//
//						TIM::Begin(0);
//
//                                                vcg::tri::io::ImporterPLY<vcgMesh>::LoadMask(name.c_str(),mask);
//
//                                                AttributeMapper am;
//                                                if(mask & vcg::tri::io::Mask::IOM_VERTCOLOR){
//                                                    m.vert.EnableColor();
//                                                    am.vert_attrs.push_back("Color4b");
//                                                }
//
//                                                vcg::tri::io::ImporterPLY<vcgMesh>::Open(m,name.c_str(),cb);
//						if(!m.face.empty())
//                                                         ocme->AddMesh(m,am);
//					}
//					else
//					{
//						 vcg::Matrix44f tra_ma;tra_ma.SetIdentity();
//						// if the file is more that 50 MB build directly from file
//						vcg::tri::io::ImporterOCMPLY<vcgMesh>::Open(m,meshona,name.c_str(),tra_ma,cb);
//
//					}
//
//				}
//
//			}
//			ocme->RemoveEmptyCells();
//}
//




int
main (int argc,char **argv )
{

//	printf("SIZE OF CELL: %d\n",sizeof(Cell));
//	printf("SIZE OF FBOOL: %d\n",sizeof(FBool));
//	printf("SIZE OF BoolVector: %d\n",sizeof(BoolVector));
//	printf("SIZE OF Chain<GIndex>: %d\n",sizeof(Chain<GIndex>));
//	printf("SIZE OF std::map<unsigned int,unsigned int>: %d\n",sizeof(std::map<unsigned int,unsigned int>));
//	printf("waste: %d\n",sizeof(FBool)*5+sizeof(BoolVector)*3+sizeof(Chain<GIndex>)+sizeof(std::map<unsigned int,unsigned int>));
//
//	printf("SIZE OF Chain<GIndex>: %d\n",sizeof(Chain<GIndex>));
//	printf("SIZE OF Chain<OFace>: %d\n",sizeof(Chain<OFace>));
//	printf("SIZE OF Chain<OVertex>: %d\n",sizeof(Chain<OVertex>));
//	printf("SIZE OF Chain<OVertex>::Chunk: %d\n",sizeof(Chain<OVertex>::Chunk));
//	printf("SIZE OF CachePolicy: %d\n",sizeof(CachePolicy));
//	printf("SIZE OF std::string: %d\n",sizeof(std::string));
//	printf("SIZE OF vcgMesh: %d\n",sizeof(vcgMesh));
//	printf("SIZE OF Impostor: %d\n",sizeof(Impostor));

//	_CrtSetBreakAlloc(1281429);

	struct stat buf;
	


{
	int c;
	int digit_optind = 0;
	unsigned int meshadded = 0,
        max_meshes = 1000 /*std::numeric_limits<unsigned int>::max()*/,
        min_meshes = 0,
        min_meshes_aln = 0,
        max_meshes_aln= 1000 /*std::numeric_limits<unsigned int>::max()*/,
        cache_memory_limit = 200,
        berkeley_page_size = 1024,
        side_factor = 50;

	std::string ocmename,tra_ma_file;
	bool logging  = false;
	bool compute_impostors  = false;
	bool overwrite_database = false;
	bool add_per_vertex_color = false;
	bool transform = false;
	bool verify = false;

        vcg::Matrix44f tra_ma;tra_ma.SetIdentity();

#ifdef _DEBUG
        printf("ocme builder DEBUG MODE\n");
#endif

	std::string stat_file = std::string(""); 
  while (1)
    {
      int this_option_optind = optind ? optind : 1;

                        c = getopt (argc, argv, "qciosvp:t:m:l:f:L:a:A:k:");
      if (c == EOF)
        break;

      switch (c)
        {
        case 'q':
          verify = true;
          break;
        case 'i':
          compute_impostors = true;
          break;

        case 't':
          transform = true;
		  printf("filename %s\n",optarg );tra_ma_file = std::string(optarg);
		  LoadTraMa(tra_ma_file.c_str(),tra_ma);
          break;

        case 's':
		  printf("filename %s\n",optarg );stat_file = std::string(optarg);
          break;

		case 'o':
          overwrite_database = true;
          break;
		case 'c':
		add_per_vertex_color = true;
		break;

		case 'f':
		printf("filename %s\n",optarg );ocmename = std::string(optarg);
          break;

		case 'a':
		 min_meshes_aln = atoi(optarg);
          break;

		case 'k':
		 side_factor = atoi(optarg);
          break;

		case 'A':
		 max_meshes_aln = atoi(optarg);
          break;

		case 'l':
		 min_meshes = atoi(optarg);
          break;

		case 'L':
		 max_meshes = atoi(optarg);
          break;

		case 'm':
		  cache_memory_limit = 	atoi(optarg);
		  break;

		case 'p':
		  berkeley_page_size = 	atoi(optarg);
		  break;

		case 'v':
			logging = true;
		  break;

		default:
          printf("unknown parameter 0%o ??\n", c);
		  UsageExit();
        }
    }

 /* .................................. */
  Impostor::Gridsize()= 8;

  if(stat_file.empty()) 
	  stat_file = ocmename + std::string("_log.txt");
  
  lgn = new Logging(stat_file.c_str());
  lgn->Append("starting");
  lgn->Push();
  

   TIM::Begin(1);
   STAT::Begin(N_STAT);

 	meshona = new OCME();
	meshona->params.side_factor = side_factor;
 	meshona->oce.cache_policy->memory_limit  = cache_memory_limit * (1<<20);

	lgn->off = !logging;
	if(!verify){
 		meshona->streaming_mode = true;	


 		int start = clock();
 		int totalstart = start;

	  if(overwrite_database)
		  remove(( ocmename+std::string(".socm")).c_str());
	 
	 #ifndef NO_BERKELEY
		if(overwrite_database)
			meshona->Create(( ocmename+std::string(".ocm")).c_str(),berkeley_page_size);
		else
			meshona->Open(( ocmename+std::string(".ocm")).c_str());
	 #else
	  if(overwrite_database)
		meshona->Create((std::string(ocmename)/*+std::string(".socm")*/).c_str(),berkeley_page_size);
	  else
		meshona->Open((std::string(ocmename)+std::string(".socm")).c_str());

	 #endif



		for(int i  = optind+min_meshes; (i < argc)&&  (i<optind+max_meshes)&& !Interrupt();++i){
		
			unsigned int wh = std::string(argv[i]).find(std::string(".aln"));

			if(wh == std::string(argv[i]).length()-4)
			
			// it is an aln file
			{
				std::vector<std::pair<std::string,vcg::Matrix44f> > aln;
				LoadAln(argv[i],aln);

				printf("aln.size()  = %d\n",aln.size());
				for(unsigned int idm = min_meshes_aln; (idm<  std::min(max_meshes_aln,aln.size())) && !Interrupt();++idm){
									STAT::Reset();
									start = clock();
					vcgMesh m;
					stat(aln[idm].first.c_str(),&buf);


					++meshona->stat.n_files;
					meshona->stat.input_file_size+=buf.st_size;
					TIM::Begin(0);
					vcg::tri::io::ImporterPLY<vcgMesh>::Open(m,aln[idm].first.c_str());
					TIM::End(0);
					meshona->stat.n_triangles += m.fn;
					meshona->stat.n_vertices += m.fn;				

					printf("%d of %d in %f sec\n",idm,aln.size(),(clock()-start)/float(CLOCKS_PER_SEC));
					vcg::tri::UpdatePosition<vcgMesh>::Matrix(m,aln[idm].second);
					unsigned int newstart = clock();
					if(!m.face.empty()){
 		 				meshona->AddMesh(m);
						++meshadded;
					}
					printf("Sizeof meshona->oce %d\n", meshona->oce.SizeOfMem());
					samples.push_back(Sample( m.fn,(clock()-newstart)/float(CLOCKS_PER_SEC),meshona->cells.size(),
															  STAT::V(0),STAT::V(1),STAT::V(6),STAT::V(3),STAT::V(5)));
					PrintFacesSec();
				}
			}
			else
				// It is a ply file
			{

				STAT::Reset();
				start = clock();
 				vcgMesh m;
				unsigned long n_faces  =0;
	 
				sprintf(lgn->Buf(),"Adding mesh %s (%d of %d)..Loading",argv[i],i-optind, argc-optind);
				lgn->Push();

				stat(argv[i],&buf);
				meshona->stat.input_file_size+=buf.st_size;
							if(buf.st_size < 500 * (1<<20)){// if the file is less that 50MB load the mesh in memory and then add it

					int mask = 0;

					TIM::Begin(0);

									AttributeMapper am;
									vcg::tri::io::ImporterPLY<vcgMesh>::LoadMask(argv[i],mask);

									if(mask & vcg::tri::io::Mask::IOM_VERTCOLOR){
										m.vert.EnableColor();
										am.vert_attrs.push_back("Color4b");
									}

					vcg::tri::io::ImporterPLY<vcgMesh>::Open(m,argv[i],cb);

					meshona->stat.n_triangles += m.fn;
					meshona->stat.n_vertices += m.vn;

					if(transform){
						sprintf(lgn->Buf(),"Apply transform" );
						lgn->Push();
						vcg::tri::UpdatePosition<vcgMesh>::Matrix(m,tra_ma);
	
					vcg::tri::io::ExporterPLY<vcgMesh>::Save(m,std::string(argv[i]).append("T.ply").c_str());
					}
					TIM::End(0);	

					n_faces = m.fn;
					sprintf(lgn->Buf(),"loaded in %f seconds\n",(clock()-start)/float(CLOCKS_PER_SEC));
					lgn->Push();

					assert( MemDbg::CheckHeap(0));

					++meshona->stat.n_files;
					
					if(!m.face.empty()) {					 
											 meshona->AddMesh(m,am);
						++meshadded;
					}
					sprintf(lgn->Buf(),"#cells: %d \n", meshona->cells.size());
					lgn->Push();
				}
				else
				{
					TIM::Begin(2);
					// if the file is more that 50 MB build directly from file
					n_faces = vcg::tri::io::ImporterOCMPLY<vcgMesh>::Open(m,meshona,argv[i],tra_ma,cb);
					TIM::End(2);
				}
				
				sprintf(lgn->Buf(),"added in %f seconds\n",(clock()-start)/float(CLOCKS_PER_SEC));
							lgn->Push();
							samples.push_back(Sample( n_faces,(clock()-start)/float(CLOCKS_PER_SEC),meshona->cells.size(),
													  STAT::V(0),STAT::V(1),STAT::V(6),STAT::V(3),STAT::V(5)));
				PrintFacesSec();
							printf("accesses % d \n",STAT::V(N_ACCESSES));

			}

		}
			meshona->RemoveEmptyCells();
 			start = clock();
			printf("number of cells: %d \n number of chains %d\n",
				meshona->cells.size(),
				meshona->oce.chains.size());

			if(compute_impostors)
				meshona->ComputeImpostors();

			meshona->Close(true);
			
			TIM::End(1);

			PrintFacesSec();

			printf("closed %ld seconds\n",(clock()-start)/CLOCKS_PER_SEC);
			printf("total %ld seconds\n",(clock()-totalstart)/CLOCKS_PER_SEC);
	//
			sprintf(lgn->Buf()," \
	n_triangles\t \t \t %12lu \n \
	n_vertices\t \t \t %12lu \n \
	size_inputMB\t \t \t %12lu \n \
	n_cells\t \t \t %10lu \n \
	n_chains\t \t \t %10lu \n \
	n_chunks_faces\t \t \t %10lu (%f per cell)\n \
	n_chunks_vertex\t \t \t %10lu (%f per cell)\n \
	size_faces\t \t \t %10lu \n \
	size_vertex\t \t \t %10lu \n \
	size_dependences\t \t \t %10lu \n \
	size_lcm_table\t \t \t %10lu \n \
	size_ocme_table\t \t \t %10lu \n \
	size_impostors\t \t \t %10lu \n \
	total size:\t \t \t %10lu \n \
	n get cells\t \t \t %15lu \n \
	input load time\t \t \t %f \n \
	add time\t \t \t \t %f \n \
	  upbox\t \t \t \t %f \n \
	  addface\t \t \t \t %f \n \
	  samples\t \t \t \t %f \n \
	total time\t \t \t %f \n \
	",
	meshona->stat.n_triangles,
	meshona->stat.n_vertices,
	meshona->stat.input_file_size,
	meshona->stat.n_cells ,
				meshona->stat.n_chains,
				meshona->stat.n_chunks_faces,meshona->stat.n_chunks_faces_avg_per_cell,
				meshona->stat.n_chunks_vertex,meshona->stat.n_chunks_vertex_avg_per_cell,

				meshona->stat.size_faces,
				meshona->stat.size_vertex,

				meshona->stat.size_dependences,
				meshona->stat.size_lcm_allocation_table,
				meshona->stat.size_ocme_table,
				meshona->stat.size_impostors,
				meshona->stat.TotalSize(),
				meshona->stat.n_getcell,
				TIM::Total(0)/float(CLOCKS_PER_SEC),
				TIM::Total(20)/float(CLOCKS_PER_SEC),
				TIM::Total(21)/float(CLOCKS_PER_SEC),
				TIM::Total(22)/float(CLOCKS_PER_SEC),
				TIM::Total(23)/float(CLOCKS_PER_SEC),
//				TIM::Total(12)/float(CLOCKS_PER_SEC),
//				TIM::Total(13)/float(CLOCKS_PER_SEC),
				TIM::Total(1)/float(CLOCKS_PER_SEC)
				);
			lgn->Push();
}
else
{
	meshona->Open(( ocmename+std::string(".socm")).c_str());
	meshona->Verify();
	meshona->Close(false);
}

		delete meshona;
		delete lgn;
}	
		
		samples.clear();


		MemDbg::End();
		MemDbg::DumpMemoryLeaks();
}

