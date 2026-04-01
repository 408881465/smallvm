// Simple build script: copy static files from src/ to dist/
// Capacitor will handle plugin registration via native layer

import fs from 'fs';
import path from 'path';

const srcDir = './src';
const distDir = './dist';

function copyDirRecursive(src, dest) {
  if (!fs.existsSync(dest)) {
    fs.mkdirSync(dest, { recursive: true });
  }
  const entries = fs.readdirSync(src, { withFileTypes: true });
  for (const entry of entries) {
    const srcPath = path.join(src, entry.name);
    const destPath = path.join(dest, entry.name);
    if (entry.isDirectory()) {
      copyDirRecursive(srcPath, destPath);
    } else {
      fs.copyFileSync(srcPath, destPath);
      console.log(`Copied: ${entry.name}`);
    }
  }
}

console.log('Building MicroBlocks web app...');

if (fs.existsSync(distDir)) {
  fs.rmSync(distDir, { recursive: true });
  console.log('Cleaned dist/');
}

copyDirRecursive(srcDir, distDir);

console.log('Build completed: src/ -> dist/');
