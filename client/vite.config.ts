import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react-swc'
import { vanillaExtractPlugin } from '@vanilla-extract/vite-plugin'
import { viteStaticCopy } from 'vite-plugin-static-copy'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [
    react(),
    vanillaExtractPlugin(),
    viteStaticCopy({
      targets: [
        {
          src: 'node_modules/libflacjs/dist/libflac.wasm.wasm',
          dest: './'
        }
      ]
    })
  ],
})
