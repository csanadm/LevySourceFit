// Standard Library
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// ROOT Core
#include <TCanvas.h>
#include <TFile.h>
#include <TF1.h>
#include <TGraph2D.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TH2.h>
#include <TLine.h>
#include <TMath.h>
#include <TVector3.h>

// ROOT Graphics and UI
#include <TLatex.h>
#include <TLegend.h>
#include <TEllipse.h>

// ROOT Math
#include <Math/Factory.h>
#include <Math/Functor.h>
#include <TMatrixDSym.h>
#include <TMatrixDSymEigen.h>

// Levy reader
#include "Levy_proj_reader.h"

using namespace std;

Levy_reader* myLevy_reader;

double fit_min = 1.;
double fit_max =50.;

const int NDIR = 3;
const int NPAR = NDIR + 2; //N, alpha, Rout, Rside, Rlong
int NDF; // For tracking the number of degrees of freedom in the fit

std::vector<TH1*> histograms;

double LevyProj1DFunc(const double *x, const double *par)
{
  double alpha = par[0];
  double R = par[1];
  double N = par[2];
  double Rcc = (R*pow(2.,1./alpha));
  return (2.*N/Rcc)*(myLevy_reader->getValue_1d(alpha, x[0]/Rcc));
}

double logLikelihood(const double *params)
{
  NDF = 0;
  double logL = 0.0;
  double alpha  = params[0];
  double N  = params[4];
  for (int idir = 0; idir < NDIR; idir++)
  {
    double *thispar = new double[NDIR];
    thispar[0] = alpha;
    thispar[1] = params[idir+1]; // idir = 0,1,2 -> Rout,Rside,Rlong; but Rout=param[1], Rside=param[2], Rlong=param[3];
    thispar[2] = N;
    double integral = histograms[idir]->Integral();
    for (int ibin = 1; ibin <= histograms[idir]->GetNbinsX(); ++ibin)
    {
      double x = histograms[idir]->GetXaxis()->GetBinCenter(ibin);
      if(x > fit_max) continue;
      if(x < fit_min) continue;
      double binVolume = ( histograms[idir]->GetXaxis()->GetBinWidth(ibin) );
      double observed = histograms[idir]->GetBinContent(ibin);
      double expected = LevyProj1DFunc(&x, thispar)*binVolume*integral;
      if(expected <= 0.) continue; // Avoid log(0) or negative expected values
      if(observed != 0.)
        logL += expected + observed*log(observed/expected) - observed;
      NDF++;
    }
    delete thispar;
  }
  NDF -= 5;
  return 2.0 * logL;
}

int main(int argc, char** argv)
{
	// Initialize Levy reader object
  myLevy_reader = new Levy_reader("levy_proj3D_values.dat"); // Download this file from https://csanad.web.elte.hu/phys/levy_proj3D_values.dat
	
	// Obtain histograms from data file
	TFile* infile = new TFile("AuAu_7p7_drho_merged.root");
	histograms.push_back((TH1*)infile->Get("D_out_LCMS_KT3"));
	histograms.push_back((TH1*)infile->Get("D_side_LCMS_KT3"));
	histograms.push_back((TH1*)infile->Get("D_long_LCMS_KT3"));

  // Create the minimizer
  ROOT::Math::Minimizer* minimizer = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad");

  // Set the minimizer properties
  minimizer->SetMaxFunctionCalls(10000);
  minimizer->SetMaxIterations(10000);
  minimizer->SetTolerance(0.001);
  
  // Create the function to be minimized
  ROOT::Math::Functor functor(&logLikelihood, NPAR);
  minimizer->SetFunction(functor);
	
  // Set the initial parameters and their steps
  minimizer->SetLimitedVariable(0, "alpha", 1.5, 0.01, 0.5, 2.0);
  minimizer->SetLimitedVariable(1, "Rout",  5.0, 0.01, 2.0, 12.0);
  minimizer->SetLimitedVariable(2, "Rside", 5.0, 0.01, 2.0, 12.0);
  minimizer->SetLimitedVariable(3, "Rlong", 5.0, 0.01, 2.0, 12.0);
  minimizer->SetVariable(4, "N", 1., 0.01);
	
  // Minimize the function
  minimizer->Minimize();
  minimizer->PrintResults();
	
  // Save the results
  const double *params = minimizer->X();
  const double *errors = minimizer->Errors();
  const double chi2val = minimizer->MinValue();
  double CL = TMath::Prob(chi2val, NDF);
	cerr << "chi^2/NDF = " << chi2val << "/" << NDF << " -> C.L. = " << CL << endl;
	
	// Create canvas for plotting
  TCanvas *c1 = new TCanvas("c1", "", 1000, 1000);
  c1->SetLogy();
  c1->SetLogx();
  c1->SetLeftMargin(0.13);
	
	// ROOT functions for drawing the fit
  TF1* f_levy_func_fitted[NDIR];
  TF1* f_levy_func_fullrange[NDIR];
	
	// Actual plotting in each direction
	const char* rhoDirString[NDIR] = {"out", "side", "long"};
	for(int idir = 0; idir<NDIR; idir++)
	{
		// Rescale histogram for plotting
    double integral = histograms[idir]->Integral();
    for (int ibin = 1; ibin <= histograms[idir]->GetNbinsX(); ibin++)
    {
      double content = histograms[idir]->GetBinContent(ibin);
      double error = histograms[idir]->GetBinError(ibin);
      double binwidth = histograms[idir]->GetXaxis()->GetBinWidth(ibin);
      histograms[idir]->SetBinContent(ibin, content / binwidth);
      histograms[idir]->SetBinError(ibin, error / binwidth);
    }
    histograms[idir]->Scale(1.0 / integral);
		
    // Set histogram plotting properties and draw it
    histograms[idir]->SetTitle("");
    histograms[idir]->GetXaxis()->SetTitle(Form("|#rho_{LCMS}^{%s}|",rhoDirString[idir]));
    histograms[idir]->GetYaxis()->SetTitle(Form("D(#rho_{LCMS}^{%s})",rhoDirString[idir]));
    histograms[idir]->GetYaxis()->SetTitleOffset(1.5);
    histograms[idir]->SetStats(0);
    histograms[idir]->GetYaxis()->SetRangeUser(1e-6, 1e-0);
    histograms[idir]->GetXaxis()->SetRangeUser(1, 200.);
    histograms[idir]->SetMarkerColor(kBlue);
    histograms[idir]->SetMarkerSize(1);
    histograms[idir]->SetMarkerStyle(8);
    histograms[idir]->SetLineColor(kBlue);
    histograms[idir]->GetYaxis()->SetTitleSize(0.04);
    histograms[idir]->GetXaxis()->SetTitleSize(0.04);
    histograms[idir]->Draw("pe");
    
    // Create a new function with the fitted parameters
		f_levy_func_fitted[idir] = new TF1(Form("f_levy_func_fitted_dir%d",idir), LevyProj1DFunc, fit_min, fit_max, NPAR-2);
    f_levy_func_fitted[idir]->SetParameters(params[0], params[idir+1], params[4]); // alpha, (Rout or Rside or Rlong), N
    f_levy_func_fitted[idir]->SetLineStyle(1); 
    f_levy_func_fitted[idir]->SetLineWidth(4);
    f_levy_func_fitted[idir]->SetLineColor(kRed);
    f_levy_func_fitted[idir]->Draw("same");
    
		// Another function to be drawin in the full range
	  f_levy_func_fullrange[idir] = new TF1(Form("f_levy_func_fullrange_dir%d",idir), LevyProj1DFunc, 1., 200., NPAR-2);
    f_levy_func_fullrange[idir]->SetParameters(params[0], params[idir+1], params[4]); // alpha, (Rout or Rside or Rlong), N
    f_levy_func_fullrange[idir]->SetLineStyle(2);
    f_levy_func_fullrange[idir]->SetLineWidth(4);
    f_levy_func_fullrange[idir]->SetLineColor(kRed);
    f_levy_func_fullrange[idir]->Draw("same");
    
		// Draw parameters on the plot
    TLatex l;
    l.SetNDC();
    l.SetTextSize(0.035);
    l.DrawLatex(0.2,0.5,  Form("#alpha = %.2f #pm %.2f",params[0],errors[0]));
    l.DrawLatex(0.2,0.45, Form("R_{%s} = (%.2f #pm %.2f) fm",rhoDirString[idir], params[idir+1], errors[idir+1]));
    l.DrawLatex(0.2,0.40, Form("#chi^{2} / NDF = %.2f / %d",chi2val,NDF));
    l.DrawLatex(0.2,0.35, Form("C.L.: %.2f%%",CL * 100));
    
    TLegend *legend = new TLegend(0.65, 0.75, 0.9, 0.9); 
    legend->SetTextSize(0.04);
    legend->AddEntry(histograms[idir], Form("D(#rho_{LCMS}^{%s})",rhoDirString[idir]), "ep");
    legend->AddEntry(f_levy_func_fitted[idir], "Levy fit", "l");
    legend->Draw("same");
    c1->Print(Form("rhist_%s.pdf",rhoDirString[idir]));
  }
	delete c1;
  return 0;
}



