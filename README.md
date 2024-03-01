# Scatterplot plugin for ManiVault  [![Actions Status](https://github.com/ManiVaultStudio/Scatterplot/actions/workflows/build.yml/badge.svg)](https://github.com/ManiVaultStudio/Scatterplot/actions)

High-performance scatterplot for the [ManiVault](https://github.com/ManiVaultStudio/core) framework, capable of handling millions of data points.

```bash
git clone git@github.com:ManiVaultStudio/Scatterplot.git
```

## Features

- 3 Render modes: Scatter, density and contouring
- Set which data dimensions are shown on the x- and y-axis on the fly
- Adjust point size, opacity and color
- Use scalars from other datasets (with same number of points) to set point size, opacity and color
- Several selection modes (rectangle, lasso, brush)
- Create subsets from selections
- Create clusters from selections

## Gallery

<p align="middle">
  <img align="top" src="https://github.com/ManiVaultStudio/Scatterplot/assets/58806453/1ec4c359-3587-4d55-b1be-6bf08eac0a65" width="30%" />
  <img align="top" src="https://github.com/ManiVaultStudio/Scatterplot/assets/58806453/ef7a4a42-67f1-47c3-b916-ccccaf097a09" width="30.85%" /> 
  <img align="top" src="https://github.com/ManiVaultStudio/Scatterplot/assets/58806453/0fb5c9ef-a16e-448e-88ca-4e84c0cfdabc" width="32%" /> </br>
  Render modes: Scatter, density and contouring
</p>

<p align="middle">
    <img align="top" src="https://github.com/ManiVaultStudio/Scatterplot/assets/58806453/8d4f8221-06d5-47ac-92ef-fc98afd88208" width="60%" /> </br>
    When dragging a dataset from the ManiVault data hierarchy into a scatter plot and the number of points match with the currently displayed data, you can choose to the dragged dataset for coloring the displayed points, chaning their size or opacity.
</p>

<p align="middle">
  <img align="top" src="https://github.com/ManiVaultStudio/Scatterplot/assets/58806453/8447cca1-e064-4365-a116-0bfd8be43fa3" width="32%" /> 
  <img align="top" src="https://github.com/ManiVaultStudio/Scatterplot/assets/58806453/ca11d532-0a9d-491c-9e99-6d3882ff5c11" width="32%" />
  <img align="top" src="https://github.com/ManiVaultStudio/Scatterplot/assets/58806453/a741b7f6-b59d-48ae-8d56-fad146a383fd" width="32%" /> </br>
  Several colormap options (e.g. using scalar values of a specific dimension form the displayed or another dataset, or 2d colormaps based on the point layout), diverse selection options (replace, add or substruct using lasso, brush or rectangle) that are updated live in other data views, and plotting options (adjust point size and opacity for the scatter mode, as well as kernel width for density and contour modes) 
</p>
