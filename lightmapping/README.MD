# notice
 - here **material.specular_**, I use GL_TEXTURE1 for sampling. It means that the specular value of middle positions(which is wood material) are vec3(0.0);

- because of **GL_TEXTURE1** is black color in middle postions, so **vec3 specularVec = light.specular_ * specularValue * texture(material.specular_, TexCoords).rgb** will be 0(zero).

- so, **middle positions(GL_TEXTURE1)** just has ambient value and diffuse value.
