import React, { useEffect, useState } from 'react'

export default function App() {
  const [presets, setPresets] = useState([])
  const [status, setStatus] = useState('Loading...')

  useEffect(() => {
    fetch('/api/presets').then(r => r.json()).then(d => {
      setPresets(d.presets || [])
      setStatus(`Ready - ${d.total || 0} presets`)
    }).catch(e => setStatus(`Error: ${e.message}`))
  }, [])

  const play = async (name) => {
    await fetch(`/api/presets/${encodeURIComponent(name)}/play`, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ duration: 2.0 }) })
  }

  return (
    <div style={{ fontFamily: 'sans-serif', padding: 16 }}>
      <h1>One-Click Sound</h1>
      <p>{status}</p>
      <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fill, minmax(220px, 1fr))', gap: 12 }}>
        {presets.map(p => (
          <div key={p.name} style={{ border: '1px solid #ddd', borderRadius: 8, padding: 12 }}>
            <div style={{ fontWeight: 600 }}>{p.name}</div>
            <div style={{ color: '#666', fontSize: 12 }}>{p.category}</div>
            <p style={{ fontSize: 14 }}>{p.description}</p>
            <button onClick={() => play(p.name)}>Play</button>
          </div>
        ))}
      </div>
    </div>
  )
}
