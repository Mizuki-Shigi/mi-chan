//🐈🐇🐈🐇

// Run: root -l -b -q /sphenix/user/mizuki/plot_cluster_z_79512_00028.C
// Electromagnetic calorimeter cluster z only; no selections applied.
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TROOT.h>
#include <iostream>
#include <memory>
#include <stdexcept>

void plot_cluster_z_79512_00028()
{
    gROOT->SetBatch(kTRUE);
    const char *inputPath = "/sphenix/user/mizuki/ana_79512_00028.root";
    const char *outputPath = "/sphenix/user/mizuki/cluster_z_79512_00028.root";
    std::unique_ptr<TFile> input(TFile::Open(inputPath, "READ"));
    if (!input || input->IsZombie())
        throw std::runtime_error("Cannot open input ROOT file");

    // Read the latest saved cycle; each vector element is one EMCal cluster.
    auto *tree = input->Get<TTree>("caloTree");
    if (!tree || !tree->GetBranch("z"))
        throw std::runtime_error("Missing caloTree/z");

    // Create the histogram in memory before opening the output file.
    gROOT->cd();
    TH1D histogram("h_cluster_z_calo",
                   "Electromagnetic calorimeter clusters;Cluster z [cm];Clusters / 0.1 cm",
                   3000, -150., 150.);
    const Long64_t clusters = tree->Draw("z>>h_cluster_z_calo", "", "goff");
    if (clusters < 0 || static_cast<Long64_t>(histogram.GetEntries()) != clusters)
        throw std::runtime_error("Histogram filling failed");
    histogram.SetDirectory(nullptr);

    std::unique_ptr<TFile> output(TFile::Open(outputPath, "RECREATE"));
    if (!output || output->IsZombie())
        throw std::runtime_error("Cannot create output ROOT file");
    output->cd();
    if (histogram.Write() <= 0)
        throw std::runtime_error("Cannot write histogram");
    output->Close();

    std::unique_ptr<TFile> check(TFile::Open(outputPath, "READ"));
    if (!check || check->IsZombie())
        throw std::runtime_error("Cannot reopen output ROOT file");
    auto *saved = check->Get<TH1D>("h_cluster_z_calo");
    if (!saved || saved->GetNbinsX() != 3000 ||
        saved->GetXaxis()->GetXmin() != -150. ||
        saved->GetXaxis()->GetXmax() != 150. ||
        saved->GetEntries() != histogram.GetEntries() ||
        check->GetNkeys() != 1)
        throw std::runtime_error("Output validation failed");
    for (int bin = 0; bin <= 3001; ++bin) {
        if (saved->GetBinContent(bin) != histogram.GetBinContent(bin))
            throw std::runtime_error("Saved bin content differs");
    }
    std::cout << "Saved: " << outputPath << '\n'
              << "Events: " << tree->GetEntries() << '\n'
              << "EMCal clusters: " << clusters << '\n'
              << "In range: " << static_cast<Long64_t>(histogram.Integral()) << '\n'
              << "Underflow: " << histogram.GetBinContent(0) << '\n'
              << "Overflow: " << histogram.GetBinContent(3001) << std::endl;
}
