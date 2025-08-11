# GitHub Pages Setup for VV-DSP Documentation

This guide explains how to set up automatic Doxygen documentation deployment to GitHub Pages.

## Prerequisites

1. **Repository Settings**: Go to your GitHub repository settings
2. **Pages Section**: Navigate to Settings → Pages
3. **Source Configuration**: Select "GitHub Actions" as the source

## Workflow Configuration

The documentation is automatically built and deployed using the `.github/workflows/docs.yml` workflow:

- **Triggers**: Runs on push to `main` branch and pull requests
- **Build**: Generates Doxygen documentation using the project's `Doxyfile`
- **Deploy**: Automatically deploys to GitHub Pages (main branch only)

## Setup Steps

### 1. Enable GitHub Pages

1. Go to your repository on GitHub
2. Click **Settings** tab
3. Scroll down to **Pages** section
4. Under **Source**, select **GitHub Actions**
5. Save the configuration

### 2. Repository Permissions

The workflow requires the following permissions (already configured in `docs.yml`):

```yaml
permissions:
  contents: read
  pages: write
  id-token: write
```

### 3. First Deployment

After pushing the workflow file to the `main` branch:

1. The workflow will automatically trigger
2. Monitor progress in the **Actions** tab
3. Once complete, documentation will be available at:
   `https://[username].github.io/[repository-name]/`

## Configuration Details

### Workflow Features

- **Conditional Deployment**: Only deploys from the `main` branch
- **Jekyll Bypass**: Creates `.nojekyll` file to prevent Jekyll processing
- **Artifact Caching**: Efficient build and deployment process
- **Error Handling**: Proper job dependencies and conditional execution

### Customization Options

#### Custom Domain (Optional)

To use a custom domain, uncomment and modify the CNAME creation in the workflow:

```yaml
# Create CNAME file if custom domain is needed
echo "docs.your-domain.com" > docs/html/CNAME
```

#### Build Configuration

The workflow includes CMake configuration to ensure all dependencies are available:

```yaml
- name: Configure project
  run: |
    cmake -S . -B build -DVV_DSP_BUILD_TESTS=OFF
```

## Monitoring and Troubleshooting

### Check Build Status

- **GitHub Actions Tab**: Monitor workflow execution
- **Badge in README**: Shows current build status
- **Pull Request Checks**: Documentation builds are validated on PRs

### Common Issues

1. **Pages Not Enabled**: Ensure GitHub Pages is configured to use "GitHub Actions"
2. **Permissions**: Verify the repository has Pages deployment permissions
3. **Branch Protection**: Ensure the workflow can write to the repository

### Debug Information

Check the workflow logs for:

- Doxygen generation output
- File artifact creation
- Deployment success/failure messages

## Accessing Documentation

Once deployed, the documentation is available at:

- **Public Repository**: `https://[username].github.io/[repository-name]/`
- **Private Repository**: Available to repository collaborators only

## Updating Documentation

Documentation automatically updates when:

- Code is pushed to the `main` branch
- Header files with Doxygen comments are modified
- The `Doxyfile` configuration is updated

No manual intervention is required - the system maintains itself!
