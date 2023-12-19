import Flac from 'libflacjs/dist/libflac.wasm.js'

const bufferTime = 0.3

export class Player {
    decoder: number = 0
    receivedData: ArrayBuffer[] = []

    currentData: ArrayBuffer | null = null
    currentDataReadPos = 0

    audioContext: AudioContext | null = null
    socket: WebSocket | null = null

    isLibFlacReady = false;

    isFlacMagicReceived = false

    isFirstTimePlay = true
    startContextTime = 0
    startSampleNumber = 0
    
    bufferOffset = 0

    onInfo?: (volume: number[]) => void;

    constructor(onReady: (player: Player) => void) {
        Flac.on("ready", (_: Flac.ReadyEvent) => {
            this.isLibFlacReady = true
            onReady(this)
        })
    }

    initDecoder() {
        this.decoder = Flac.create_libflac_decoder(false)
        if(this.decoder === 0) {
            console.error("Failed to create decoder")
            return
        }
        const status_decoder = Flac.init_decoder_stream(this.decoder, this.onFlacReadCallback.bind(this), this.onFlacWriteCallback.bind(this), this.onFlacErrorCallback.bind(this), this.onFlacMetadataCallback.bind(this))
        if(status_decoder !== 0) {
            console.error("Failed to init decoder")
            return
        }
    }

    unlockAudio() {
        this.audioContext?.resume()
    }

    private initAudioContext(sampleRate: number) {
        if(this.audioContext) {
            this.audioContext.close()
        }
        this.audioContext = new AudioContext({sampleRate: sampleRate})
    }

    start() {
        this.socket = new WebSocket(`ws://${window.location.host}/ws`)
        // this.socket = new WebSocket(`ws://localhost:8000/ws`)
        this.socket.addEventListener("message", this.onWebSocketMessage.bind(this))
    }

    stop() {
        this.socket?.close()
        if(this.isLibFlacReady && this.decoder > 0)
            Flac.FLAC__stream_decoder_finish(this.decoder)
        this.audioContext?.close()
    }

    async onWebSocketMessage(e: MessageEvent) {
        this.receivedData.push(await e.data.arrayBuffer())
        if(this.isFlacMagicReceived) {
            const flac_return = Flac.FLAC__stream_decoder_process_single(this.decoder)
            const state = Flac.FLAC__stream_decoder_get_state(this.decoder)
            console.log(flac_return, state)
        }
        this.isFlacMagicReceived = true
    }

    private onFlacReadCallback: Flac.decoder_read_callback_fn = (numberOfBytes: number) => {
        if(!this.currentData) {
            this.currentData = this.receivedData.splice(0, 1)[0]
        }

        if(numberOfBytes < this.currentData.byteLength - this.currentDataReadPos) {
            const data = this.currentData.slice(this.currentDataReadPos, this.currentDataReadPos + numberOfBytes)
            this.currentDataReadPos += numberOfBytes
            return {buffer: new Uint8Array(data), readDataLength: numberOfBytes, error: false}
        } else {
            const data = this.currentData.slice(this.currentDataReadPos)
            numberOfBytes = data.byteLength
            this.currentData = null
            this.currentDataReadPos = 0
            return {buffer: new Uint8Array(data), readDataLength: numberOfBytes, error: false}
        }
    }

    private onFlacWriteCallback: Flac.decoder_write_callback_fn = (channelsBuffer: Uint8Array[], frameInfo: Flac.BlockMetadata) => {
        if(!this.audioContext) return
        if(this.isFirstTimePlay) {
            this.isFirstTimePlay = false
            this.startContextTime = this.audioContext?.currentTime ?? 0
            this.startSampleNumber = frameInfo.number
        }

        const source = this.audioContext.createBufferSource()
        const buffer = this.audioContext.createBuffer(frameInfo.channels, frameInfo.blocksize, frameInfo.sampleRate)
        const peakVolumes: number[] = []
        for (let i = 0; i < frameInfo.channels; i++) {
            let peakVolume = 0
            const view = new DataView(channelsBuffer[i].buffer)
            const len = channelsBuffer[i].length / 2
            const buf32 = new Float32Array(len)
            for(let i = 0; i < len; i++) {
                buf32[i] = view.getInt16(i * 2, true) / 0x7fff
                if(peakVolume < Math.abs(buf32[i]))
                    peakVolume = Math.abs(buf32[i])
            }
            buffer.copyToChannel(buf32, i)
            // in dB
            peakVolumes.push(Math.log10(peakVolume) * 10)
        }
        source.buffer = buffer
        source.connect(this.audioContext.destination)

        const ellapsedSample = frameInfo.number - this.startSampleNumber
        let scheduleTime = this.startContextTime + ellapsedSample / frameInfo.sampleRate + bufferTime + this.bufferOffset

        if(scheduleTime < this.audioContext.currentTime) {
            this.bufferOffset += this.audioContext.currentTime - scheduleTime + bufferTime
            scheduleTime = this.startContextTime + ellapsedSample / frameInfo.sampleRate + bufferTime + this.bufferOffset
        }

        source.start(scheduleTime)

        setTimeout(() => {
            if(this.onInfo) this.onInfo(peakVolumes)
        }, (scheduleTime - this.audioContext.currentTime) * 1000)
    }

    private onFlacMetadataCallback: Flac.metadata_callback_fn = (metadata?: Flac.StreamMetadata) => {
        const sampleRate = metadata?.sampleRate ?? 44100
        this.initAudioContext(sampleRate)
    }

    private onFlacErrorCallback: Flac.decoder_error_callback_fn = (errorCode: number, errorDescription: Flac.FLAC__StreamDecoderErrorStatus) => {
        console.error('decode error callback', errorCode, errorDescription);
    }
}