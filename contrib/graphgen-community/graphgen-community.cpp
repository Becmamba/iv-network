#include <bitset>
#include "gen-community.h"
#include "InterestTagGenerator.h"
#include "InterestTagGenerator2.h"
#include "PAPool.h"
#include "Snap.h"
int main(int argc, char* argv[]) {
//  PAPool p;
//  p.setWeight(0, 1000);
//  p.setWeight(1, 500);
//  
//  
//  for (int i = 0; i < 300; ++i) {
//    printf("%d\n", p.randomPick());
//  }
//  
//  
//  
//  printf("OK\n");
//  
//  return 0;
  
  
  Env = TEnv(argc, argv, TNotify::StdNotify);
  Env.PrepArgs(TStr::Fmt("Graph generators. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
  TExeTm ExeTm;
  Try
  const TStr Plot = Env.GetIfArgPrefixStr("-g:", "i", "Which generator to use:"
                                          "\n\ti: Interest Tag Generation. Required parameters: \n"
                                          "\t\tn (number of nodes), k (edges created by each new node), t (num of tags), f (main interest factor)\n"
                                          );
  const int numNodes = Env.GetIfArgPrefixInt("-n:", 1000, "Number of nodes");
  const int numOutDeg = Env.GetIfArgPrefixInt("-k:", 3, "Degree");
  const int numTags = Env.GetIfArgPrefixInt("-t:", 5, "Number of interest fields");
  const double Pn1 = Env.GetIfArgPrefixFlt("-pn1:", 0.85, "P(n=1)");
  const int mainInterestFactor = Env.GetIfArgPrefixInt("-f:", 100, "Main interest factor");
  const int interval = Env.GetIfArgPrefixInt("-i:", 1000, "Output interval");
  const int interestsChangeInterval = Env.GetIfArgPrefixInt("-c:",1000,"interests change inteval");
  if (Env.IsEndOfRun()) { return 0; }

  TExeTm ExeTm;
  TInt::Rnd.PutSeed(0); // initialize random seed
  printf("Generating...\n");



  GenComm::InterestTagGenerator generator(numNodes, numOutDeg, numTags, mainInterestFactor, Pn1);
  generator.setInterval(interval);
  generator.setInterestsChangeInterval(interestsChangeInterval);
  generator.run();

  //  GenComm::GenInterestTag(5000, 3, 5, 100);


  Catch
  printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
  return 0;
}
