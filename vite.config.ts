import { defineConfig } from 'vite';
import { resolve } from 'path';
import fs from 'fs';

export default defineConfig({
  root: './src',
  build: {
    outDir: '../dist',
    minify: false,
    emptyOutDir: true,
    rollupOptions: {
      input: {
        main: './src/index.html',
        microblocks: './src/microblocks.html'
      }
    }
  },
  plugins: [{
    name: 'copy-files',
    writeBundle() {
      // Copy gp_wasm files
      const wasmFiles = ['gp_wasm.js', 'gp_wasm.wasm', 'gp_wasm.data'];
      wasmFiles.forEach(file => {
        try {
          fs.copyFileSync(
            resolve(__dirname, `chromeApp/webapp/${file}`),
            resolve(__dirname, `dist/${file}`)
          );
          console.log(`Copied: ${file}`);
        } catch (err) {
          console.error(`Error copying ${file}:`, err);
        }
      });

      // Copy boardie directory
      const srcBoardieDir = resolve(__dirname, 'chromeApp/webapp/boardie');
      const destBoardieDir = resolve(__dirname, 'dist/boardie');
      if (fs.existsSync(srcBoardieDir)) {
        copyDirRecursive(srcBoardieDir, destBoardieDir);
        console.log('Boardie directory copied successfully');
      }

      // Copy icons directory
      const srcIconsDir = resolve(__dirname, 'chromeApp/webapp/icons');
      const destIconsDir = resolve(__dirname, 'dist/icons');
      if (fs.existsSync(srcIconsDir)) {
        copyDirRecursive(srcIconsDir, destIconsDir);
        console.log('Icons directory copied successfully');
      }
    }
  }]
});

function copyDirRecursive(src, dest) {
  if (!fs.existsSync(dest)) {
    fs.mkdirSync(dest, { recursive: true });
  }
  const entries = fs.readdirSync(src, { withFileTypes: true });
  for (const entry of entries) {
    const srcPath = resolve(src, entry.name);
    const destPath = resolve(dest, entry.name);
    if (entry.isDirectory()) {
      copyDirRecursive(srcPath, destPath);
    } else {
      fs.copyFileSync(srcPath, destPath);
    }
  }
}
