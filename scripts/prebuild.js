// prebuild.js
// Copies gp_wasm files from chromeApp/webapp to src before build
import fs from 'fs';
import path from 'path';

const srcDir = 'chromeApp/webapp';
const destDir = 'src';

const files = ['gp_wasm.js', 'gp_wasm.wasm', 'gp_wasm.data'];

files.forEach(file => {
  const src = path.join(srcDir, file);
  const dest = path.join(destDir, file);
  if (fs.existsSync(src)) {
    fs.copyFileSync(src, dest);
    console.log(`Copied: ${file}`);
  } else {
    console.warn(`Warning: ${file} not found in ${srcDir}`);
  }
});

console.log('Prebuild completed.');
