
import { container, playButton, title } from './app.css'
import { usePlayer } from './player/hook'
import { Volume } from './volume/volume'
const App: React.FC = () => {

  const { start , status, peakVolumes} = usePlayer()

  return <div className={`${container}`}>
    <h1 className={title}>JuceFlacStreamingDemo</h1>

    <div>
      {
         <Volume volumes={peakVolumes} />
      }
    </div>

    <button className={playButton} onClick={start} disabled={status !== "ready"}>      
      {
        {
          "loading": "Wait",
          "ready": "Play",
          "playing": "Playing..."
        }[status]
      }
    </button>

  </div>
}

export default App