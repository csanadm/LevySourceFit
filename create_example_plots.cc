#include "Levy_proj_reader.h"
#include <iostream>
#include <TFile.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TF1.h>
#include <TROOT.h>
#include <TStyle.h>

void create_example_plots() {
    gStyle->SetOptStat(0);
    // Create a Levy_reader object
    Levy_reader* levy_reader = new Levy_reader("levy_proj3D_values.dat");

    // Create a canvas for the 1D plot
    TCanvas* c1 = new TCanvas("c1", "Angular average of 3D Levy distribution", 800, 600);
    c1->SetLogy();

    // Create a histogram for the 1D plot
    TH1D* h_1D_avg = new TH1D("h_1D_avg", "Angular average of 3D Levy distribution;q (GeV/c);C(q)", 100, 0, 2);
    h_1D_avg->Draw();

    // Create the 1D function
    TF1* f_1D_avg = new TF1("f_1D_avg", [&](const double* x, const double* p) {
        double q = x[0];
        double lambda = p[0];
        double R = p[1];
        double alpha = p[2];
        return 1.0 + lambda * levy_reader->getValue_1d(alpha, q * R);
    }, 0, 2, 3);
    f_1D_avg->SetParameters(0.5, 0.4, 1.2);
    f_1D_avg->SetParNames("#lambda", "R", "#alpha");
    f_1D_avg->Draw("same");

    // Save the 1D plot
    c1->SaveAs("levy_1Davg_example.pdf");

    // Create a canvas for the 3D projections
    TCanvas* c2 = new TCanvas("c2", "3D Levy distribution projections", 1200, 400);
    c2->Divide(3, 1);

    // Create histograms for the 3D projections
    TH1D* h_out = new TH1D("h_out", "out projection;q_{out} (GeV/c);C(q_{out})", 100, 0, 1);
    TH1D* h_side = new TH1D("h_side", "side projection;q_{side} (GeV/c);C(q_{side})", 100, 0, 1);
    TH1D* h_long = new TH1D("h_long", "long projection;q_{long} (GeV/c);C(q_{long})", 100, 0, 1);

    // Create the 3D functions
    TF1* f_out = new TF1("f_out", [&](const double* x, const double* p) {
        double q = x[0];
        double lambda = p[0];
        double R = p[1];
        double alpha = p[2];
        return 1.0 + lambda * levy_reader->getValue_3d(alpha, q * R);
    }, 0, 1, 3);
    f_out->SetParameters(0.5, 0.4, 1.2);
    f_out->SetParNames("#lambda", "R_{out}", "#alpha");

    TF1* f_side = new TF1("f_side", [&](const double* x, const double* p) {
        double q = x[0];
        double lambda = p[0];
        double R = p[1];
        double alpha = p[2];
        return 1.0 + lambda * levy_reader->getValue_3d(alpha, q * R);
    }, 0, 1, 3);
    f_side->SetParameters(0.5, 0.3, 1.2);
    f_side->SetParNames("#lambda", "R_{side}", "#alpha");


    TF1* f_long = new TF1("f_long", [&](const double* x, const double* p) {
        double q = x[0];
        double lambda = p[0];
        double R = p[1];
        double alpha = p[2];
        return 1.0 + lambda * levy_reader->getValue_3d(alpha, q * R);
    }, 0, 1, 3);
    f_long->SetParameters(0.5, 0.6, 1.2);
    f_long->SetParNames("#lambda", "R_{long}", "#alpha");

    // Draw the plots
    c2->cd(1);
    h_out->Draw();
    f_out->Draw("same");
    c2->cd(2);
    h_side->Draw();
    f_side->Draw("same");
    c2->cd(3);
    h_long->Draw();
    f_long->Draw("same");

    // Save the 3D plot
    c2->SaveAs("levy_proj3D_example.pdf");
}

int main() {
    gROOT->SetBatch(kTRUE);
    create_example_plots();
    return 0;
}
