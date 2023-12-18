
import { container } from './app.css'
import { Player } from './player'
const App: React.FC = () => {
  return <div className={`${container}`}>
    hello
    <button onClick={() => {
      new Player((player) => {
        player.initDecoder()
        player.start()
        player.unlockAudio()
      })
    }}>play</button>
  </div>
}

export default App