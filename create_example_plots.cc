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

// Levy reader
#include "Levy_proj_reader.h"

using namespace std;

Levy_reader* myLevy_reader;

double LevyAvg1DFunc(const double *x, const double *par)
{
  double alpha = par[0];
  double R = par[1];
  double N = par[2];
  double Rcc = (R*pow(2.,1./alpha));
  return (2.*N/Rcc)*(myLevy_reader->getValue_3d(alpha, x[0]/Rcc));
}

double LevyProj1DFunc(const double *x, const double *par)
{
  double alpha = par[0];
  double R = par[1];
  double N = par[2];
  double Rcc = (R*pow(2.,1./alpha));
  return (2.*N/Rcc)*(myLevy_reader->getValue_1d(alpha, x[0]/Rcc));
}

int main(int argc, char** argv)
{
	// Initialize Levy reader object
  myLevy_reader = new Levy_reader("levy_proj3D_values.dat");

  // Create canvas for plotting
  TCanvas *c1 = new TCanvas("c1", "", 1000, 1000);
  c1->SetLogy();
  c1->SetLogx();
  c1->SetLeftMargin(0.13);

  // --- 1D average plot ---
  TF1* f_levy_1Davg = new TF1("f_levy_1Davg", LevyAvg1DFunc, 0.1, 200., 3);
  f_levy_1Davg->SetParameters(1.2, 5.0, 1.0); // alpha, R, N
  f_levy_1Davg->SetTitle("1D Angular Average of 3D Levy Distribution");
  f_levy_1Davg->GetXaxis()->SetTitle("|#rho_{LCMS}|");
  f_levy_1Davg->GetYaxis()->SetTitle("D(#rho_{LCMS})");
  f_levy_1Davg->Draw();
  c1->Print("levy_1Davg_example.png");

  // --- 3D projection plot ---
  TCanvas *c2 = new TCanvas("c2", "", 1000, 1000);
  c2->SetLogy();
  c2->SetLogx();
  c2->SetLeftMargin(0.13);

  TF1* f_levy_proj_out = new TF1("f_levy_proj_out", LevyProj1DFunc, 0.1, 200., 3);
  f_levy_proj_out->SetParameters(1.2, 6.0, 1.0); // alpha, Rout, N
  f_levy_proj_out->SetLineColor(kRed);
  f_levy_proj_out->SetTitle("1D Projections of 3D Levy Distribution");
  f_levy_proj_out->GetXaxis()->SetTitle("|#rho_{o,s,l}|");
  f_levy_proj_out->GetYaxis()->SetTitle("D(#rho_{o,s,l})");
  f_levy_proj_out->Draw();

  TF1* f_levy_proj_side = new TF1("f_levy_proj_side", LevyProj1DFunc, 0.1, 200., 3);
  f_levy_proj_side->SetParameters(1.2, 5.0, 1.0); // alpha, Rside, N
  f_levy_proj_side->SetLineColor(kBlue);
  f_levy_proj_side->Draw("same");

  TF1* f_levy_proj_long = new TF1("f_levy_proj_long", LevyProj1DFunc, 0.1, 200., 3);
  f_levy_proj_long->SetParameters(1.2, 4.0, 1.0); // alpha, Rlong, N
  f_levy_proj_long->SetLineColor(kGreen);
  f_levy_proj_long->Draw("same");

  TLegend *legend = new TLegend(0.15, 0.15, 0.65, 0.45);
  legend->SetTextSize(0.04);
  legend->AddEntry(f_levy_proj_out, "D(#rho_{out}), R_{out} = 6.0 fm", "l");
  legend->AddEntry(f_levy_proj_side, "D(#rho_{side}), R_{side} = 5.0 fm", "l");
  legend->AddEntry(f_levy_proj_long, "D(#rho_{long}), R_{long} = 4.0 fm", "l");
  legend->Draw("same");

  c2->Print("levy_proj3D_example.png");

  delete c1;
  delete c2;

  return 0;
}
