import * as React from 'react'
import * as S from './style' // 假设这里定义了基础容器
import classnames from '$web-common/classnames'
import WebAnimationPlayer from '../../api/web_animation_player'
import DataContext from '../../state/context'
import { shouldPlayAnimations } from '../../state/hooks'

interface BackgroundProps {
  children?: JSX.Element
  static: boolean
  onLoad?: () => void
}

function Background (props: BackgroundProps) {
  const ref = React.useRef<HTMLDivElement>(null)
  const { setScenes } = React.useContext(DataContext)
  const [hasLoaded, setHasLoaded] = React.useState(false)
  const isReadyForAnimation = hasLoaded && !props.static

  React.useEffect(() => {
    if (!ref.current || !isReadyForAnimation) return

    const s1 = new WebAnimationPlayer()
    const s2 = new WebAnimationPlayer()

    const orb1 = ref.current.querySelector('.orb-1')
    const orb2 = ref.current.querySelector('.orb-2')
    const orb3 = ref.current.querySelector('.orb-3')
    const grid = ref.current.querySelector('.tech-grid')

    s1.to(orb1, { transform: 'translate(-10%, -10%) scale(1.2)', filter: 'blur(40px)' })
      .to(orb2, { transform: 'translate(10%, 20%) scale(1.1)', opacity: 0.6 })
      .to(grid, { opacity: 0.4, transform: 'perspective(500px) rotateX(20deg)' })

    s2.to(orb1, { transform: 'translate(-30%, 10%) scale(1.5)', filter: 'blur(60px)' })
      .to(orb2, { transform: 'translate(-20%, -20%) scale(1.3)' })
      .to(orb3, { transform: 'translate(15%, 15%) scale(1.4)', opacity: 0.8 })
      .to(grid, { transform: 'perspective(500px) rotateX(35deg) translateY(-50px)', opacity: 0.2 })

    setScenes({ s1, s2 })
  }, [isReadyForAnimation])

  React.useEffect(() => {
    if (!ref.current || !isReadyForAnimation) return

    const s1 = new WebAnimationPlayer()
    const container = ref.current.querySelector('.visual-layers')

    s1.to(container, { opacity: 1 }, { delay: 300 })
    
    const lastAnimationEl = s1.animations[s1.animations.length - 1]
    lastAnimationEl?.addEventListener('finish', () => props.onLoad?.())
    s1.play()
  }, [isReadyForAnimation])

  React.useEffect(() => {
    const timer = setTimeout(() => {
      setHasLoaded(true)
      if (!shouldPlayAnimations) {
        props.onLoad?.()
      }
    }, 100)
    return () => clearTimeout(timer)
  }, [])

  return (
    <S.Box ref={isReadyForAnimation ? ref : null} style={containerStyle}>
      <div className={classnames('visual-layers', { 'is-visible': hasLoaded })} style={layerWrapperStyle}>
        <div className="tech-grid" style={gridStyle} />
        
        <div className="orb-1" style={{ ...orbBase, background: 'radial-gradient(circle, #4c1d95 0%, transparent 70%)', top: '-10%', left: '-10%', width: '60vw', height: '60vw' }} />
        <div className="orb-2" style={{ ...orbBase, background: 'radial-gradient(circle, #1e40af 0%, transparent 70%)', bottom: '10%', right: '-5%', width: '50vw', height: '50vw' }} />
        <div className="orb-3" style={{ ...orbBase, background: 'radial-gradient(circle, #701a75 0%, transparent 70%)', top: '20%', right: '15%', width: '40vw', height: '40vw' }} />
      </div>

      <div className="content-box" style={{ position: 'relative', zIndex: 10 }}>
        {props.children}
      </div>
    </S.Box>
  )
}


const containerStyle: React.CSSProperties = {
  backgroundColor: '#050505', 
  overflow: 'hidden',
  position: 'relative',
  width: '100vw',
  height: '100vh'
}

const layerWrapperStyle: React.CSSProperties = {
  position: 'absolute',
  inset: 0,
  opacity: 0,
  transition: 'opacity 1s ease-in-out'
}

const orbBase: React.CSSProperties = {
  position: 'absolute',
  borderRadius: '50%',
  filter: 'blur(50px)',
  pointerEvents: 'none',
  zIndex: 1
}

const gridStyle: React.CSSProperties = {
  position: 'absolute',
  width: '200%',
  height: '200%',
  top: '-50%',
  left: '-50%',
  backgroundImage: `
    linear-gradient(to right, rgba(255,255,255,0.05) 1px, transparent 1px),
    linear-gradient(to bottom, rgba(255,255,255,0.05) 1px, transparent 1px)
  `,
  backgroundSize: '60px 60px',
  transform: 'perspective(500px) rotateX(15deg)',
  zIndex: 0,
  pointerEvents: 'none'
}

export default Background